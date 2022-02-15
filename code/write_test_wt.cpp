#include <unistd.h>
#include <assert.h>
#include <thread>
#include <string>
#include <iostream>
#include <cstdio>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <wiredtiger.h>
#include <dirent.h>
#include <sys/stat.h>
#include "random.h"
#ifndef TOTAL_NUM
#define TOTAL_NUM 10000000LL
#endif
#define EXPAND_QUEUE_SIZE TOTAL_NUM / 10 + 1

#ifndef QUEUE_NUM
#define QUEUE_NUM 8
#endif
#define key_size_ 28
#define SEED 14664
#define BATCH_SIZE 1


using namespace std;

struct async_requests
{
    int seq;
    int key;
    int batch_pos;
    timespec start;
    async_requests(int s, int k) : seq(s), key(k)
    {
    }
    async_requests() : seq(-1), key(-1)
    {
    }
};
mutex mt;
int tail[QUEUE_NUM];
async_requests qs[QUEUE_NUM][TOTAL_NUM / QUEUE_NUM + EXPAND_QUEUE_SIZE];
int ns[QUEUE_NUM][TOTAL_NUM / QUEUE_NUM];

long duration_ns(timespec start, timespec end)
{
    long ns;
    ns = end.tv_sec * 1000000000L + end.tv_nsec - start.tv_sec * 1000000000L - start.tv_nsec;
    return ns;
}
void GenerateKeyFromInt(uint64_t v, char *key)
{
    char *start = key;
    char *pos = start;
    int bytes_to_fill = std::min(key_size_, 8);
    for (int i = 0; i < bytes_to_fill; ++i)
    {
        pos[i] = (v >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
    }
    pos += bytes_to_fill;
    if (key_size_ > pos - start)
    {
        memset(pos, '0', key_size_ - (pos - start));
    }
}
enum WriteMode
{
    RANDOM,
    SEQUENTIAL,
    UNIQUE_RANDOM
};
class KeyGenerator
{
public:
    KeyGenerator(rocksdb::Random64 *rand, WriteMode mode, uint64_t num,
                 uint64_t /*num_per_set*/ = 64 * 1024)
        : rand_(rand), mode_(mode), num_(num), next_(0)
    {
        if (mode_ == UNIQUE_RANDOM)
        {
            values_.resize(num_);
            for (uint64_t i = 0; i < num_; ++i)
            {
                values_[i] = i;
            }
            rocksdb::RandomShuffle(values_.begin(), values_.end(),
                                   static_cast<uint32_t>(SEED));
        }
    }

    uint64_t Next()
    {
        switch (mode_)
        {
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
    rocksdb::Random64 *rand_;
    WriteMode mode_;
    const uint64_t num_;
    uint64_t next_;
    std::vector<uint64_t> values_;
};

void worker_thread(int queue_seq, WT_CURSOR *cursor)
{
    char *value_str = new char[100];
    memset(value_str, '0', 100);
    value_str[99]='\0';
    timespec start, end;
    char key[key_size_+1];
    int seq = 0;
    bool endflag = 0;
    int batch_size = 0;
    int t,ret;

    clock_gettime(CLOCK_REALTIME, &start);
    while (!endflag)
    {
        t = tail[queue_seq];
        if (seq >= t)
        {
            this_thread::yield();
            //   this_thread::sleep_for(chrono::microseconds(1));
            continue;
        }
        for (batch_size = 0; seq < t && batch_size < BATCH_SIZE; seq++, batch_size++)
        {
            if (qs[queue_seq][seq].seq == -1)
            {
                endflag = true;
                break;
            }
            // GenerateKeyFromInt(qs[queue_seq][seq].key, key);
            sprintf(key,"%028d",qs[queue_seq][seq].key);
            // clock_gettime(CLOCK_REALTIME,&(qs[queue_seq][seq].start));
            cursor->set_key(cursor, key); /* Insert a record. */
            cursor->set_value(cursor, value_str);
            ret = cursor->insert(cursor);
            if (ret)
            {
                cout << "error\n" << endl;
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    auto atime = duration_ns(start, end);
    int batch_size_distribution[BATCH_SIZE];
    double total_time = 0;
    for (int i = 0; i < BATCH_SIZE; i++)
    {
        batch_size_distribution[i] = 0;
    }
    for (int i = 0; i < seq; i++)
    {
        // cout << "batch_pos:"<< qs[queue_seq][i].batch_pos <<" ,"<< (int)ns[i] <<"ns"<<endl;
        int batchpos = qs[queue_seq][i].batch_pos;
        // if(batchpos==32)cout <<"32positon: "<<i<<endl;
        batch_size_distribution[batchpos - 1]++;
        total_time += ns[queue_seq][i];
    }
    sort(ns[queue_seq], ns[queue_seq] + seq);
    mt.lock();
    // cout <<endl<<"thread " << queue_seq <<"result"<<endl;
    // for(int i=0;i<BATCH_SIZE;i++){
    //     cout<<i+1<<":"<<batch_size_distribution[i]<<",";
    // }
    // cout << endl<<"avgtime:"<< (int)(total_time/(seq)) <<"ns" << endl;
    // cout << "percentile: 1st:"<< ns[queue_seq][0] <<"ns,50th:"<<  ns[queue_seq][int(0.5*seq)] <<"ns,90th:" << ns[queue_seq][int(0.9*seq)] <<"ns,99th:" <<ns[queue_seq][int(0.99*seq)] << "ns,99.9th:"<<ns[queue_seq][int(0.999*seq)] << "ns,99.99th:" <<ns[queue_seq][int(0.9999*seq)]<<"ns"<<endl;
    cout << "thread " << queue_seq << " has processed " << seq << " requests. Avg latency: " << atime / seq << "ns, QPS: " << 1000000000LL * seq / atime << ", throughputs: " << 1000000000LL * 128 / 1024 / 1024 * seq / atime << "MB/s" << endl;
    // cout << stats<<endl;
    mt.unlock();
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("please enter the directory of db_path as paramater \n(e.g.: read_test /mnt/optanessd/db)");
        return 0;
    }
    //db init
    const string DBPath = argv[1];
    WT_CONNECTION *db[QUEUE_NUM];
    WT_SESSION *session[QUEUE_NUM];
    WT_CURSOR *cursor[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        char c[16];
        std::sprintf(c, "%03d", i);
        if(0 != access((DBPath + c).c_str(),0)){
            mkdir((DBPath + c).c_str(),0755);
        }
        wiredtiger_open((DBPath + c).c_str(), NULL, "create", &db[i]);//open gai
    
        db[i]->open_session(db[i], NULL, NULL, &session[i]);
        session[i]->create(session[i], "table:access", "key_format=S,value_format=S");
        session[i]->open_cursor(session[i], "table:access", NULL, NULL, &cursor[i]);
    }

    //rng
    rocksdb::Random64 *rand = new rocksdb::Random64(SEED);
    KeyGenerator keygen(rand, SEQUENTIAL, TOTAL_NUM);
    //thread allocation
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        tail[i] = 0;
    }
    //generate workloads
    timespec start, middle, end;
    thread wts[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        wts[i] = thread(worker_thread, i, cursor[i]);
    }
    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i < TOTAL_NUM; i++)
    {
        uint64_t key = keygen.Next();
        int q_num = key % QUEUE_NUM;
        qs[q_num][tail[q_num]].seq = i;
        qs[q_num][tail[q_num]].key = key;
        clock_gettime(CLOCK_REALTIME, &qs[q_num][tail[q_num]].start);
        tail[q_num]++;
    }
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        qs[i][tail[i]].seq = -1;
        qs[i][tail[i]].key = -1;
        tail[i]++;
    }
    clock_gettime(CLOCK_REALTIME, &middle);
    cout << "finish" << endl;
    for (int j = 0; j < QUEUE_NUM; j++)
    {
        wts[j].join();
    }
    clock_gettime(CLOCK_REALTIME, &end);
    for (int j = 0; j < QUEUE_NUM; j++)
    {
        db[j]->close(db[j],NULL);//close gai
    }
    uint64_t atime, btime;
    atime = duration_ns(start, middle);
    btime = duration_ns(start, end);

    cout << "loading requests............." << endl;
    cout << "loading time per request (avg):" << atime / TOTAL_NUM << "ns, QPS:" << 1000000000LL * TOTAL_NUM / atime << ", throughputs:" << 1000000000LL * 128 / 1024 / 1024 * TOTAL_NUM / atime << "MB/s" << endl;

    cout << "processing requests:............" << endl;
    cout << "total time (avg):" << btime / TOTAL_NUM << "ns, QPS:" << 1000000000LL * TOTAL_NUM / btime << ", throughputs:" << 1000000000LL * 128 / 1024 / 1024 * TOTAL_NUM / btime << "MB/s" << endl;
    return 0;
}