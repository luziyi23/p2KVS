#include<unistd.h>
#include<assert.h>
#include<thread>
#include<string>
#include<iostream>
#include<cstdio>
#include<mutex>
#include<atomic>
#include<cstring>
#include<rocksdb/slice.h>
#include<rocksdb/db.h>
#include<rocksdb/options.h>
#include<rocksdb/table.h>
#include<rocksdb/filter_policy.h>
#include"random.h"
#include"merge_iter.h"
#define TOTAL_NUM 320000000LL
#define READ_NUM 200000
#define EXPAND_QUEUE_SIZE TOTAL_NUM/100
#define QUEUE_NUM 8
#define key_size_ 28
#define SEED 4321
#define BATCH_SIZE 32
#define MULTITHREAD_SCAN


/*
    读请求
    point query：分配到请求队列直接查询
    scan：顺序读负载

    本程序只能处理读请求point query，并且考虑使用multiget减缓压力
*/


using namespace std;

struct async_requests
{
    int seq;
    int key;
    short scan_size;
    bool write;
    async_requests():seq(0),key(0),scan_size(0),write(false){
    }
};
struct scan_results{
    vector<string> result[QUEUE_NUM];
    // int n_[QUEUE_NUM];
    rocksdb::Iterator* iter[QUEUE_NUM];
    bool scan_over[QUEUE_NUM];
    int scan_size;
};
mutex mt;
atomic<int> tail[QUEUE_NUM];
async_requests qs[QUEUE_NUM][TOTAL_NUM/QUEUE_NUM+EXPAND_QUEUE_SIZE];
scan_results scans[READ_NUM];
bool recycle[READ_NUM];
bool recycles[QUEUE_NUM][READ_NUM];
string value=string('0',100);
class Multi_db_iterator{
    private:
        rocksdb::DB** dbs;
    public:
        Multi_db_iterator(rocksdb::DB* dbs[]):dbs(dbs){
            rocksdb::ReadOptions options(true,true);
            
        }
};
unsigned long duration_ns(timespec start,timespec end){
    unsigned long ns;
    ns=end.tv_sec*1000000000LL+end.tv_nsec-start.tv_sec*1000000000LL-start.tv_nsec;
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
                      static_cast<uint32_t>(1000));
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

/*
*   读请求也作为单独的读来处理
*   考虑是否做聚合？
*   
*/
void worker_thread(int queue_seq,rocksdb::DB* db){
    timespec start,end;
    char key[key_size_];
    rocksdb::Slice k(key,key_size_);
    vector<string> values(BATCH_SIZE);
    int seq=0;
    bool endflag=0;
    int batch_size=0;
    string result;
    auto readoptions=rocksdb::ReadOptions(true,true);
    vector<rocksdb::Slice> keys;
    rocksdb::Status s;
    int found=0,total_scan=0;
    int scan_times=0;
    clock_gettime(CLOCK_REALTIME,&start);
    while(!endflag){
        while(seq>=tail[queue_seq]){
          // this_thread::yield();
          this_thread::sleep_for(chrono::nanoseconds(10));
        }
        if(qs[queue_seq][seq].seq==-1){
            endflag=true;
            break;
        }
        async_requests a=qs[queue_seq][seq];
        GenerateKeyFromInt(a.key,key);
        rocksdb::Iterator* iter=db->NewIterator(readoptions);
        total_scan+=a.scan_size;
        iter->Seek(k);
        if(iter->Valid() && a.scan_size){
            found++;
            int i=1;
            scans[seq].result[queue_seq].push_back(iter->key().ToString());
            for(;i<a.scan_size;i++){
                // cout << iter->key().ToString() <<" "<< int(iter->key().data()[6])<<","<< int(iter->key().data()[7]) << endl;
                iter->Next();
                if(!iter->Valid())break;
                scans[seq].result[queue_seq].push_back(iter->key().ToString());
                found++;
            }
        }
        scans[seq].iter[queue_seq]=iter;
        scans[seq].scan_over[queue_seq]=true;
        scan_times++;
        seq++;
    }  
    clock_gettime(CLOCK_REALTIME,&end);
    auto atime=duration_ns(start,end);
    string stats;
    db->GetProperty("rocksdb.levelstats",&stats); 
    mt.lock();
    cout <<"线程"<<queue_seq<<"处理请求数"<<seq<<"个，找到"<<found<<"/"<<total_scan<<"个平均处理时间："<<atime/seq <<"ns, QPS："<<1000000000LL*seq/atime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*found/atime <<"MiB/s" <<endl;
    // cout << stats <<endl;
    mt.unlock();  	
}

class vector_merge_iter
{
private:
    int now[QUEUE_NUM];
    Iterator* db_iters[QUEUE_NUM];
    int current_;
    vector<string>* children_;
    const rocksdb::Comparator* comparator_;
public:
    vector_merge_iter(int num):children_(scans[num].result),current_(0),comparator_(rocksdb::BytewiseComparator()){
        for(int i=0;i<QUEUE_NUM;i++){
            db_iters[i]=scans[num].iter[i];
            now[i]=0;
        }  
        FindSmallest();
    };
    vector_merge_iter():current_(0),comparator_(rocksdb::BytewiseComparator()){

    }
    ~vector_merge_iter(){
    };
    void recycle(){
        for(int i=0;i<QUEUE_NUM;i++)
            delete db_iters[i];
    }
    void init(int num){
        children_=scans[num].result;
        for(int i=0;i<QUEUE_NUM;i++){
            now[i]=0;
            if(children_[i].size()==0){
                now[i]=-1;
            }
            db_iters[i]=scans[num].iter[i];
        }
        FindSmallest();
    }
    rocksdb::Slice get_key(int i){
        if(now[i]!=-1){
            return children_[i][now[i]];
        }
        if(!db_iters[i]->Valid())return NULL;
        return db_iters[i]->key();
    }

    void Next(){
        if(now[current_]!=-1){
            now[current_]++;
            if(now[current_]>=children_[current_].size()){
                now[current_]=-1;
            }
        }else{
            db_iters[current_]->Next();
        }
        FindSmallest();
    };
    int Current(){
        return current_;
    }
    void FindSmallest(){
        int smallest=0;
        rocksdb::Slice smallest_key=get_key(0);
        for (int i = 1; i < QUEUE_NUM; i++) {
            rocksdb::Slice key= get_key(i);
            if(key!=NULL){
                if(smallest==NULL){
                    smallest=i;
                }else if (comparator_->Compare(key, smallest_key) < 0) {
                    smallest = i;
                    smallest_key=key;
                }
            }
        }
        current_=smallest;
    };
    
    rocksdb::Slice key(){
        return get_key(current_);
    };
    bool Valid(){
        if(now[current_]<children_[current_].size()){
            return true;
        }else if(now[current_]==-1){
            return db_iters[current_]->Valid();
        }
    }
};

void merge_thread(){
    int seq=0;
    int found=0;
    timespec start,end;
    vector_merge_iter* mergeiter=new vector_merge_iter();
    clock_gettime(CLOCK_REALTIME,&start);
    while (seq<READ_NUM)
    {
        for(int i=0;i<QUEUE_NUM;i++){
            while (scans[seq].scan_over[i]==false)
            {
                this_thread::yield();
            }
        }
        int i=0;
        mergeiter->init(seq);
        int scan_size=scans[seq].scan_size;
        for(i=1;i<scan_size;i++){
            mergeiter->Next();
            if(!mergeiter->Valid())break;
        }
        // mergeiter->recycle();
        // delete mergeiter;
        recycle[seq]=true;
        found+=i;
        seq++;
        // cout << seq <<"<"<<READ_NUM<<":"<<(seq<READ_NUM)<<endl;
    }
    clock_gettime(CLOCK_REALTIME,&end);
    auto atime=duration_ns(start,end);
    mt.lock();
    cout<<"归并线程处理请求数"<<seq<<"个，scanKV数量："<<found<<",平均处理时间"<<atime/seq<<"ns,QPS:"<<seq*1000000000LL/atime<<",换算为128KV吞吐率为"<<1000000000LL*128/1024/1024*found/atime<<"MiB/s"<<endl;
    mt.unlock();
}



void recycle_thread(){
    int seq=0;
    while (seq<READ_NUM)
    {
        if(recycle[seq]){
            for(int i=0;i<QUEUE_NUM;i++){
                delete scans[seq].iter[i];
            }
            seq++;
        }
        this_thread::yield();
    }
    
}
void recycle_thread2(int q_num){
    int seq=0;
    while (seq<READ_NUM)
    {
        if(recycles[q_num][seq]){
            delete scans[seq].iter[q_num];
            seq++;
        }
        this_thread::yield();
    }
}
int main(){
  //db初始化
    const string DBPath = "/mnt/optanessd/db_single_thread";
    rocksdb::DB* db[QUEUE_NUM];
    rocksdb::Options options;
    options.compression=rocksdb::kNoCompression;
    options.create_if_missing = true;
    options.max_background_jobs=88;
    rocksdb::BlockBasedTableOptions* table_options =
          reinterpret_cast<rocksdb::BlockBasedTableOptions*>(
              options.table_factory->GetOptions());
        table_options->filter_policy.reset(rocksdb::NewBloomFilterPolicy(
            10, false));
      
    for(int i=0;i<QUEUE_NUM;i++){
        char c[3];
        std::sprintf(c,"%03d",i);
        rocksdb::Status status = rocksdb::DB::Open(options,DBPath+c, &db[i]);
        assert(status.ok());
    }
    rocksdb::Random64* rand=new rocksdb::Random64(SEED);
    KeyGenerator keygen(rand,WriteMode::RANDOM,TOTAL_NUM-1000);
    KeyGenerator scansizegen(new rocksdb::Random64(SEED),WriteMode::RANDOM,49);
    rocksdb::Iterator* iters[QUEUE_NUM];
    for(int i=0;i<QUEUE_NUM;i++){
        iters[i]=db[i]->NewIterator(rocksdb::ReadOptions(true,true));
    }
  
    // MergingIterator* merge_iter=new MergingIterator(rocksdb::BytewiseComparator(),iters,QUEUE_NUM);
    // merge_iter->SeekToFirst();
    //线程分配
    #ifdef MULTITHREAD_SCAN
    for(int i=0;i<QUEUE_NUM;i++){
        tail[i]=0;
    }
    for(int i=0;i<READ_NUM;i++){
        recycle[i]=false;
        for(int j=0;j<QUEUE_NUM;j++){
            scans[i].result[j].reserve(100);
            scans[i].scan_over[j]=false;
            // recycles[j][i]=false;
        }
    }
    //先将请求全部分配
    thread wts[QUEUE_NUM];
    for(int i=0;i<QUEUE_NUM;i++){
        wts[i]= thread(worker_thread,i,db[i]);
    }
    thread mt=thread(merge_thread);
    thread ct=thread(recycle_thread);
    ct.detach();
    // thread cts[QUEUE_NUM];
    // for(int i=0;i<QUEUE_NUM;i++){
    //     cts[i]=thread(recycle_thread2,i);
    //     cts[i].detach();
    // }
    #endif
    timespec start,middle,end;
    clock_gettime(CLOCK_REALTIME,&start);
    int i,q_num=0;
    int key;
    int scan_size=100;
    int scan_per_db=13;
    for(i=0;i<READ_NUM;i++){
        key=keygen.Next();
        // scan_size=scansizegen.Next()+1;
        #ifdef MULTITHREAD_SCAN
        scans[i].scan_size=scan_size;
        for(q_num=0;q_num<QUEUE_NUM;q_num++){
            qs[q_num][tail[q_num]].seq=i;
            qs[q_num][tail[q_num]].key=key;
            qs[q_num][tail[q_num]].write=false;
            qs[q_num][tail[q_num]].scan_size=scan_per_db;
            tail[q_num]++;
        }
        #else
            //scan迭代器初始化
            rocksdb::Iterator* iters[QUEUE_NUM];
            for(int j=0;j<QUEUE_NUM;j++){
                iters[j]=db[j]->NewIterator(rocksdb::ReadOptions(true,true));
            }
            MergingIterator* merge_iter=new MergingIterator(rocksdb::BytewiseComparator(),iters,QUEUE_NUM);
            char keystr[key_size_];
            GenerateKeyFromInt(key,keystr);
            merge_iter->Seek(rocksdb::Slice(keystr,key_size_));
            for(int j=0;j<scan_size && merge_iter->Valid();j++){
                //  cout << merge_iter->key().ToString() << "  "<< unsigned(merge_iter->key().data()[6])<<","<<unsigned(merge_iter->key().data()[7])<<endl;
                merge_iter->Next();
            }
            delete merge_iter;
        #endif
    }
    cout << i << endl;
    for(int i=0;i<QUEUE_NUM;i++){
        qs[i][tail[i]].seq=-1;
        qs[i][tail[i]].key=-1;
        tail[i]++;
    }
    
    clock_gettime(CLOCK_REALTIME,&middle);  
    #ifdef MULTITHREAD_SCAN
    for(int j=0;j<QUEUE_NUM;j++){
        wts[j].join();
    }
    mt.join();
    #endif
    clock_gettime(CLOCK_REALTIME,&end);
    uint64_t atime,btime;
    atime=duration_ns(start,middle);
    btime=duration_ns(start,end);
    cout << "负载准备时间：............." << endl;
    cout <<"平均生成时间："<<atime/READ_NUM <<"ns, QPS："<<1000000000LL*READ_NUM/atime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*READ_NUM/atime <<"MB/s" <<endl;
    cout <<"两线程竞争测试:............"<<endl;
    cout <<"平均完成时间："<<btime/READ_NUM <<"ns, QPS："<<1000000000LL*READ_NUM/btime<<", 换算为128BKV的吞吐率为"<< 1000000000LL*128/1024/1024*READ_NUM*scan_size/btime <<"MB/s" <<endl;
    return 0;
}