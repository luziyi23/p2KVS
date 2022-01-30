#include <unistd.h>
#include <assert.h>
#include <thread>
#include <string>
#include <iostream>
#include <cstdio>
#include <mutex>
#include <atomic>
#include <cstring>
#include <rocksdb/slice.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/table.h>
#include <rocksdb/filter_policy.h>
#include "random.h"
#include "merge_iter.h"
#ifndef TOTAL_NUM
#define TOTAL_NUM 320000000LL
#endif
#ifndef READ_NUM
#define READ_NUM 6400000
#endif

#define EXPAND_QUEUE_SIZE TOTAL_NUM / 10000

#ifndef QUEUE_NUM
#define QUEUE_NUM 8
#endif

#define key_size_ 28
#define SEED 4321
#define BATCH_SIZE 32


using namespace std;

struct async_requests
{
    int seq;
    int key;
    enum REQUEST_TYPE : char
    {
        WRITE,
        READ,
        SCAN
    };
    REQUEST_TYPE type;
    async_requests(int s, int k, REQUEST_TYPE t) : seq(s), key(k)
    {
    }
    async_requests() : seq(0), key(0)
    {
    }
};
mutex mt;
atomic<int> tail[QUEUE_NUM];
async_requests qs[QUEUE_NUM][TOTAL_NUM / QUEUE_NUM + EXPAND_QUEUE_SIZE];

string value = string('0', 100);
class Multi_db_iterator
{
private:
    rocksdb::DB **dbs;

public:
    Multi_db_iterator(rocksdb::DB *dbs[]) : dbs(dbs)
    {
        rocksdb::ReadOptions options(true, true);
    }
};
unsigned long duration_ns(timespec start, timespec end)
{
    unsigned long ns;
    ns = end.tv_sec * 1000000000LL + end.tv_nsec - start.tv_sec * 1000000000LL - start.tv_nsec;
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
            // NOTE: if memory consumption of this approach becomes a concern,
            // we can either break it into pieces and only random shuffle a section
            // each time. Alternatively, use a bit map implementation
            // (https://reviews.facebook.net/differential/diff/54627/)
            values_.resize(num_);
            for (uint64_t i = 0; i < num_; ++i)
            {
                values_[i] = i;
            }
            rocksdb::RandomShuffle(values_.begin(), values_.end(),
                                   static_cast<uint32_t>(1000));
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

void worker_thread(int queue_seq, rocksdb::DB *db)
{
    timespec start, end;
    char key[key_size_];
    rocksdb::Slice k(key, key_size_);
    vector<string> values(BATCH_SIZE);
    int seq = 0;
    bool endflag = 0;
    int batch_size = 0;
    int t;
    string result;
    auto readoptions = rocksdb::ReadOptions(true, true);
    vector<rocksdb::Slice> keys;
    rocksdb::Status s;
    int found = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    while (!endflag)
    {
        t = tail[queue_seq];
        while (seq >= tail[queue_seq])
        {
            // this_thread::yield();
            this_thread::sleep_for(chrono::microseconds(10));
        }
        if (qs[queue_seq][seq].seq == -1)
        {
            endflag = true;
            break;
        }
        GenerateKeyFromInt(qs[queue_seq][seq].key, key);
        // s=db->Get(readoptions,k,&result);
        keys.push_back(rocksdb::Slice(key, key_size_));
        // if(s.ok()){
        // found++;
        // }
        seq++;

        //完成写入
        if (keys.size() == BATCH_SIZE)
        {
            vector<rocksdb::Status> ss = db->MultiGet(readoptions, keys, &values);
            for (rocksdb::Status s : ss)
            {
                if (s.ok())
                    found++;
            }
            keys.clear();
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    if (seq == 0)
        return;
    auto atime = duration_ns(start, end);

    mt.lock();
    cout << "Thread " << queue_seq << " has processed " << seq << "requests, find: " << found << ",Avg. latency: " << atime / seq << "ns, QPS:" << 1000000000LL * seq / atime << ", Throuputs: " << 1000000000LL * 128 / 1024 / 1024 * seq / atime << "MB/s" << endl;
    // cout << atime <<endl;
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
    rocksdb::DB *db[QUEUE_NUM];
    rocksdb::Options options;
    options.compression = rocksdb::kNoCompression;
    options.create_if_missing = true;
    options.max_background_jobs = 88;
    rocksdb::BlockBasedTableOptions *table_options =
        reinterpret_cast<rocksdb::BlockBasedTableOptions *>(
            options.table_factory->GetOptions());
    table_options->filter_policy.reset(rocksdb::NewBloomFilterPolicy(
        10, false));

    for (int i = 0; i < QUEUE_NUM; i++)
    {
        char c[3];
        std::sprintf(c, "%03d", i);
        rocksdb::Status status = rocksdb::DB::Open(options, DBPath + c, &db[i]);
        assert(status.ok());
    }
    rocksdb::Random64 *rand = new rocksdb::Random64(SEED);
    KeyGenerator keygen(rand, WriteMode::RANDOM, TOTAL_NUM);
    rocksdb::Iterator *iters[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        iters[i] = db[i]->NewIterator(rocksdb::ReadOptions(true, true));
    }

    MergingIterator *merge_iter = new MergingIterator(rocksdb::BytewiseComparator(), iters, QUEUE_NUM);
    merge_iter->SeekToFirst();
    // thread allocation
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        tail[i] = 0;
    }

    // generate workloads
    timespec start, middle, end;
    thread wts[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        wts[i] = thread(worker_thread, i, db[i]);
    }
    clock_gettime(CLOCK_REALTIME, &start);
    int i, q_num = 0;
    int key;
    for (i = 0; i < READ_NUM && merge_iter->Valid(); i++)
    {
#ifdef SEQ_READ
        //         // merge_iter->key();
        //         // cout << merge_iter->key().ToString() <<endl;

        //         // merge_iter->value();
        cout << merge_iter->key().ToString() << "  " << unsigned(merge_iter->key().data()[6]) << "," << unsigned(merge_iter->key().data()[7]) << endl;
        merge_iter->Next();
#else
        key = keygen.Next();
        q_num = key % QUEUE_NUM;
        qs[q_num][tail[q_num]].seq = i;
        qs[q_num][tail[q_num]].key = key;
        qs[q_num][tail[q_num]].type = async_requests::REQUEST_TYPE::READ;
        tail[q_num]++;
#endif
    }
    cout << i << endl;
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        qs[i][tail[i]].seq = -1;
        qs[i][tail[i]].key = -1;
        tail[i]++;
        cout << tail[i] << endl;
    }
    clock_gettime(CLOCK_REALTIME, &middle);
    for (int j = 0; j < QUEUE_NUM; j++)
    {
        wts[j].join();
    }

    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t atime, btime;
    atime = duration_ns(start, middle);
    btime = duration_ns(start, end);
    cout << "generating requests:............." << endl;
    cout << "loading time per request (avg):" << atime / READ_NUM << "ns, QPS:" << 1000000000LL * READ_NUM / atime << ", throuputs" << 1000000000LL * 128 / 1024 / 1024 * READ_NUM / atime << "MB/s" << endl;
    cout << "processing requests:............" << endl;
    cout << "request processing time (avg):" << btime / READ_NUM << "ns, QPS:" << 1000000000LL * READ_NUM / btime << ", throuputs" << 1000000000LL * 128 / 1024 / 1024 * READ_NUM / btime << "MB/s" << endl;
    return 0;
}