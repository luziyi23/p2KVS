#include<unistd.h>
#include<assert.h>
#include<thread>
#include<string>
#include<iostream>
#include<cstdio>
#include<mutex>
#include<atomic>
#include<cstring>
#include<leveldb/slice.h>
#include<leveldb/db.h>
#include<leveldb/options.h>
#include<leveldb/write_batch.h>
#include<leveldb/table.h>
#include<leveldb/filter_policy.h>
#include"random.h"
#define TOTAL_SIZE 100000000LL
#define EXPAND_QUEUE_SIZE TOTAL_SIZE/10+1
#define QUEUE_NUM 8
#define key_size_ 28
#define SEED 14664
#define BATCH_SIZE 1

/*
    请求分配为全内存模式：先分配好trace大小的内存空间，
                        运行时所有trace载入该内存，
                        以减少读取trace和创建请求时内存分配的开销。
                        子线程也只需要读取该内存区。
    生产者消费者动态模式：主线程为生产者，将trace转换为的请求对象填入预先分配好的内存区。
                        子线程为消费者，读取对应请求的内存区并执行请求。
    性能:可以达到裸性能。
*/


using namespace std;
using namespace leveldb;

struct async_requests
{
    int seq;
    int key;
    unsigned long ns;
    int batch_pos;
    timespec start;
    async_requests(int s,int k):seq(s),key(k){
    }
    async_requests():seq(-1),key(-1){
    }
};
mutex mt;
int tail[QUEUE_NUM];
async_requests qs[QUEUE_NUM][TOTAL_SIZE/QUEUE_NUM+EXPAND_QUEUE_SIZE];




long duration_ns(timespec start,timespec end){
    long ns;
    ns=end.tv_sec*1000000000L+end.tv_nsec-start.tv_sec*1000000000L-start.tv_nsec;
    return ns;
}
void GenerateKeyFromInt(uint64_t v, char* key) {
    char* start = key;
    char* pos = start;
    int bytes_to_fill = std::min(key_size_, 8);
    for (int i = 0; i < bytes_to_fill; ++i) {
        pos[i] = (v >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
    }
    pos += bytes_to_fill;
    if (key_size_ > pos - start) {
      memset(pos, '0', key_size_ - (pos - start));
    }
  }
 enum WriteMode {
    RANDOM, SEQUENTIAL, UNIQUE_RANDOM
  };
class KeyGenerator {
   public:
    KeyGenerator(rocksdb::Random64* rand, WriteMode mode, uint64_t num,
                 uint64_t /*num_per_set*/ = 64 * 1024)
        : rand_(rand), mode_(mode), num_(num), next_(0) {
      if (mode_ == UNIQUE_RANDOM) {
        // NOTE: if memory consumption of this approach becomes a concern,
        // we can either break it into pieces and only random shuffle a section
        // each time. Alternatively, use a bit map implementation
        // (https://reviews.facebook.net/differential/diff/54627/)
        values_.resize(num_);
        for (uint64_t i = 0; i < num_; ++i) {
          values_[i] = i;
        }
        rocksdb::RandomShuffle(values_.begin(), values_.end(),
                      static_cast<uint32_t>(SEED));
      }
    }

    uint64_t Next() {
      switch (mode_) {
        case SEQUENTIAL:
          return next_++;
        case RANDOM:
          return rand_->Next() % num_;
        case UNIQUE_RANDOM:
          assert(next_ < num_);
          return values_[next_++];
      }
      assert(false);
      return std::numeric_limits<uint64_t>::max();
    }

   private:
    rocksdb::Random64* rand_;
    WriteMode mode_;
    const uint64_t num_;
    uint64_t next_;
    std::vector<uint64_t> values_;
};

void worker_thread(int queue_seq,leveldb::DB* db){

    leveldb::Slice value;
    char* value_str=new char[100];
    memset(value_str,'0',100);
    value=leveldb::Slice(value_str,100);
    timespec start,end;
    char key[key_size_];
    leveldb::Slice k(key,key_size_);
    std::string getvalue;
    int seq=0;
    bool endflag=0;
    int batch_size=0;
    int t;
    auto writeoptions=leveldb::WriteOptions();
    
    leveldb::Status s;
    clock_gettime(CLOCK_REALTIME,&start);
    while(!endflag){
        t=tail[queue_seq];
        if(seq>=t){
          this_thread::yield();
        //   this_thread::sleep_for(chrono::nanoseconds(100));
        }
        WriteBatch batch;
        for(batch_size=0;seq<t && batch_size<BATCH_SIZE;seq++,batch_size++){
            if(qs[queue_seq][seq].seq==-1){
                endflag=true;
                break;
            }
            GenerateKeyFromInt(qs[queue_seq][seq].key,key);
            // clock_gettime(CLOCK_REALTIME,&(qs[queue_seq][seq].start)); 
            batch.Put(k,value);
            // s=db->Get(ReadOptions(),k,&getvalue);
            // if(!s.ok()){
            //   cout<< "error"<<endl;
            // }
        }
        
        //完成写入
        if(batch_size){
            // cout <<"write batch:" << batch_size << endl;
            s=db->Write(writeoptions,&batch);
            //  if(!s.ok()){
            //   cout<< "error"<<endl;
            // }
            // timespec endtime;
            // clock_gettime(CLOCK_REALTIME,&endtime);
            // for(int i=0;i<batch_size;i++){
            //     int pos=seq-i-1;
            //     qs[queue_seq][pos].ns=duration_ns(qs[queue_seq][pos].start,endtime);
            //     qs[queue_seq][pos].batch_pos=batch_size;
            // }
        }
    }  
    clock_gettime(CLOCK_REALTIME,&end);
    auto atime=duration_ns(start,end);
    string stats;
    db->GetProperty("level.levelstats",&stats);
    mt.lock();
    // cout <<"thread " << queue_seq<<endl;
    // int batch_size_distribution[BATCH_SIZE];
    // double total_time=0;
    // for(int i=0;i<BATCH_SIZE;i++){
    //     batch_size_distribution[i]=0;
    // }
    // for(int i=0;i<seq;i++){
    //     // cout << "batch_pos:"<< qs[queue_seq][i].batch_pos <<" ,"<< (int)qs[queue_seq][i].ns <<"ns"<<endl;
    //     int batchpos=qs[queue_seq][i].batch_pos;
    //     batch_size_distribution[batchpos-1]++;
    //     total_time+=qs[queue_seq][i].ns;
    // }
    // for(int i=0;i<BATCH_SIZE;i++){
    //     cout<<i<<":"<<batch_size_distribution[i]<<",";
    // }
    // cout << endl<<"avgtime:"<< (int)(total_time/(seq)) <<"ns" << endl;
    cout <<"线程"<<queue_seq<<"处理请求数"<<seq<<"个，平均处理时间："<<atime/seq <<"ns, QPS："<<1000000000LL*seq/atime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*seq/atime <<"MB/s" <<endl;
    // cout << stats<<endl;
    mt.unlock();  	
}
int main(){
  //db初始化
    const string DBPath = "/mnt/optanessd/db_single_thread";
    leveldb::DB* db[QUEUE_NUM];
        leveldb::Options options;
        options.compression=leveldb::kNoCompression;
        options.create_if_missing = true;
    // options.use_direct_io_for_flush_and_compaction=true;
    // options.use_direct_reads=false;
    // options.enable_pipelined_write=false;
    // options.allow_concurrent_memtable_write=false;
    // options.max_background_jobs=40;
    // options.wal_bytes_per_sync=4*1048576;
    // leveldb::BlockBasedTableOptions* table_options =
    //       reinterpret_cast<leveldb::BlockBasedTableOptions*>(
    //           options.table_factory->GetOptions());
    // table_options->filter_policy.reset(leveldb::NewBloomFilterPolicy(
        // 10));
    for(int i=0;i<QUEUE_NUM;i++){

      char c[3];
      std::sprintf(c,"%03d",i);
      leveldb::DestroyDB(DBPath+c,options);
      leveldb::Status status = leveldb::DB::Open(options,DBPath+c, &db[i]);
      assert(status.ok());
    }   
    rocksdb::Random64* rand=new rocksdb::Random64(SEED);
    KeyGenerator keygen(rand,UNIQUE_RANDOM,TOTAL_SIZE);
    //线程分配
    for(int i=0;i<QUEUE_NUM;i++){
        tail[i]=0;
    }
    //先将请求全部分配
    timespec start,middle,end;
    thread wts[QUEUE_NUM];
    for(int i=0;i<QUEUE_NUM;i++){
        wts[i]= thread(worker_thread,i,db[0]);
    }
    clock_gettime(CLOCK_REALTIME,&start);
    for(int i=0;i<TOTAL_SIZE;i++){
        uint64_t key=keygen.Next();
        int q_num=key%QUEUE_NUM;
        qs[q_num][tail[q_num]].seq=i;
        qs[q_num][tail[q_num]].key=key;
        // clock_gettime(CLOCK_REALTIME,&qs[q_num][tail[q_num]].start); 
        tail[q_num]++;
        // for(int j=0;j<100;j++)
        // {
            
        // }
    }
    for(int i=0;i<QUEUE_NUM;i++){
        qs[i][tail[i]].seq=-1;
        qs[i][tail[i]].key=-1;
        tail[i]++;
    }
    clock_gettime(CLOCK_REALTIME,&middle);  
    for(int j=0;j<QUEUE_NUM;j++){
        wts[j].join();
    }
    clock_gettime(CLOCK_REALTIME,&end);
    // for(int j=0;j<QUEUE_NUM;j++){
    //     db[j]->Close();
    // }
    uint64_t atime,btime;
    atime=duration_ns(start,middle);
    btime=duration_ns(start,end);
    cout << "负载准备时间：............." << endl;
    cout <<"平均生成时间："<<atime/TOTAL_SIZE <<"ns, QPS："<<1000000000LL*TOTAL_SIZE/atime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*TOTAL_SIZE/atime <<"MB/s" <<endl;
    cout <<"两线程竞争测试:............"<<endl;
    cout <<"平均完成时间："<<btime/TOTAL_SIZE <<"ns, QPS："<<1000000000LL*TOTAL_SIZE/btime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*TOTAL_SIZE/btime <<"MB/s" <<endl;
    return 0;
}