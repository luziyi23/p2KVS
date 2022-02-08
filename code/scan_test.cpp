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
#define READ_NUM 100000LL
#endif
#define EXPAND_QUEUE_SIZE TOTAL_NUM / 10000
#ifndef QUEUE_NUM
#define QUEUE_NUM 8
#endif
#define key_size_ 28
#define SEED 4321
#define BATCH_SIZE 32
#define MULTITHREAD_SCAN

using namespace std;

struct async_requests
{
    int seq;
    int key;
    short scan_size;
    bool write;
    async_requests() : seq(0), key(0), scan_size(0), write(false)
    {
    }
};
struct scan_results
{
    vector<rocksdb::Slice> result[QUEUE_NUM];
};
mutex mt;
atomic<int> tail[QUEUE_NUM];
async_requests qs[QUEUE_NUM][TOTAL_NUM / QUEUE_NUM + EXPAND_QUEUE_SIZE];
// scan_results scans[READ_NUM];
enum SCAN_STATUS : short
{
    SCAN_OVER,
    MERGE_START,
    NEED_ITER,
    ITER_OVER
};
SCAN_STATUS scan_status[QUEUE_NUM];
vector<rocksdb::Slice> scans[QUEUE_NUM];
bool over = false;
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
    string result;
    auto readoptions = rocksdb::ReadOptions(true, true);
    vector<rocksdb::Slice> keys;
    rocksdb::Status s;
    int found = 0, total_scan = 0;
    int scan_times = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    while (!endflag)
    {
        while (seq >= tail[queue_seq])
        {
            // this_thread::yield();
            this_thread::sleep_for(chrono::nanoseconds(10));
        }
        if (qs[queue_seq][seq].seq == -1)
        {
            endflag = true;
            break;
        }
        async_requests a = qs[queue_seq][seq];
        GenerateKeyFromInt(a.key, key);
        rocksdb::Iterator *iter = db->NewIterator(readoptions);
        total_scan += a.scan_size;
        iter->Seek(k);

        if (iter->Valid())
        {
            found++;
            int size = 0;
            string str = iter->key().ToString();
            scans[queue_seq][size] = rocksdb::Slice(str);
            size++;
            mt.lock();
            cout << queue_seq << " " << scans[queue_seq][0].ToString() << "  " << int(scans[queue_seq][0].data()[6]) << "," << int(scans[queue_seq][0].data()[7]) << endl;
            mt.unlock();
            for (int i = 1; i < a.scan_size; i++)
            {
                iter->Next();
                if (!iter->Valid())
                    break;
                string str2 = iter->key().ToString();
                scans[queue_seq][size] = rocksdb::Slice(str2);
                size++;

                found++;
            }
            // mt.lock();
            // cout << queue_seq <<" "<< scans[queue_seq][0].ToString() << "  "<< int( scans[queue_seq][0].data()[6])<<","<<int(scans[queue_seq][0].data()[7])<<endl;
            // mt.unlock();
            // mt.lock();
            // cout<<queue_seq<< "need SCAN_OVER"<<endl;
            // mt.unlock();
            scan_status[queue_seq] = MERGE_START;

            while (scan_status[queue_seq] != SCAN_OVER)
            {
                // this_thread::sleep_for(chrono::milliseconds(100));
                if (scan_status[queue_seq] == NEED_ITER)
                {
                    // mt.lock();
                    // cout<<queue_seq<< "MORE_ITER"<<endl;
                    // mt.unlock();
                    for (int i = 0; i < a.scan_size; i++)
                    {
                        iter->Next();
                        if (!iter->Valid())
                            break;
                        scans[queue_seq].push_back(rocksdb::Slice(iter->key().ToString()));
                        found++;
                    }
                    scan_status[queue_seq] = ITER_OVER;
                }
                this_thread::yield();
            }
            scans[queue_seq].clear();
        }

        scan_times++;
        delete iter;
        seq++;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    // if(seq==0)return;
    auto atime = duration_ns(start, end);
    // rocksdb.levelstats
    string stats;
    db->GetProperty("rocksdb.levelstats", &stats);
}

class vector_merge_iter
{
private:
    int now[QUEUE_NUM];
    // vector<rocksdb::Slice>* children_[QUEUE_NUM];
    int current_;
    const rocksdb::Comparator *comparator_;

public:
    vector_merge_iter() : current_(0), comparator_(rocksdb::BytewiseComparator())
    {
        for (int i = 0; i < QUEUE_NUM; i++)
        {
            now[i] = 0;
            // mt.lock();
            // cout <<" "<< scans[i][now[i]].ToString() << "  "<< int( scans[i][now[i]].data()[6])<<","<<int(scans[i][now[i]].data()[7])<<endl;
            // mt.unlock();
        }
        FindSmallest();
    };
    ~vector_merge_iter();

    bool Next()
    {
        if (scans[current_].size() <= now[current_] + 1)
            return false;
        now[current_]++;
        FindSmallest();
        return true;
    };
    int Current()
    {
        return current_;
    }
    void FindSmallest()
    {
        int smallest = 0;
        rocksdb::Slice smallest_key = scans[0][now[0]];
        for (int i = 1; i < QUEUE_NUM; i++)
        {
            rocksdb::Slice key = scans[i][now[i]];
            if (comparator_->Compare(key, smallest_key) < 0)
            {
                smallest = i;
                smallest_key = key;
            }
        }
        current_ = smallest;
    };

    rocksdb::Slice key()
    {
        return scans[current_].at(now[current_]);
    };
};

void merge_thread()
{
    int seq = 0;
    while (seq < READ_NUM)
    {
        for (int i = 0; i < QUEUE_NUM; i++)
        {
            while (scan_status[i] != MERGE_START)
            {
                // mt.lock();
                // cout<< i <<"need merge_start"<<endl;
                // mt.unlock();
                // this_thread::sleep_for(chrono::milliseconds(100));
                this_thread::yield();
            }
        }

        vector_merge_iter *mergeiter = new vector_merge_iter();
        for (int i = 1; i < 100; i++)
        {
            // cout << mergeiter->key().ToString() << "  "<< unsigned(mergeiter->key().data()[6])<<","<<unsigned(mergeiter->key().data()[7])<<endl;
            bool s = mergeiter->Next();
            if (!s)
            {

                int current = mergeiter->Current();
                scan_status[current] = NEED_ITER;
                while (scan_status[current] != ITER_OVER)
                {
                    // mt.lock();
                    // cout<< "need ITER_OVER"<<endl;
                    // mt.unlock();
                    // this_thread::sleep_for(chrono::milliseconds(100));
                    this_thread::yield();
                }
                s = mergeiter->Next();
                assert(s);
            }
        }
        for (int i = 0; i < QUEUE_NUM; i++)
        {
            scan_status[i] = SCAN_OVER;
        }
        seq++;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("please enter the directory of db_path as paramater \n(e.g.: scan_test /mnt/optanessd/db)");
        return 0;
    }
    //db init
    const string DBPath = argv[1];
    rocksdb::DB *db[QUEUE_NUM];
    rocksdb::Options options;
    options.compression = rocksdb::kNoCompression;
    options.create_if_missing = true;
    options.max_background_jobs = 40;
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
    KeyGenerator keygen(rand, WriteMode::SEQUENTIAL, TOTAL_NUM - 1000);
    KeyGenerator scansizegen(new rocksdb::Random64(SEED), WriteMode::RANDOM, 49);
    rocksdb::Iterator *iters[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        iters[i] = db[i]->NewIterator(rocksdb::ReadOptions(true, true));
    }

// MergingIterator* merge_iter=new MergingIterator(rocksdb::BytewiseComparator(),iters,QUEUE_NUM);
// merge_iter->SeekToFirst();
// thread allocation
#ifdef MULTITHREAD_SCAN
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        tail[i] = 0;
    }
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        scan_status[i] = SCAN_OVER;
        scans[i].reserve(100);
        // for(int j=0;j<QUEUE_NUM;j++){
        //     scans[i].result[j].resize(100);
        // }
    }
    //generate workloads
    thread wts[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        wts[i] = thread(worker_thread, i, db[i]);
    }
    thread mt = thread(merge_thread);
#endif
    timespec start, middle, end;
    clock_gettime(CLOCK_REALTIME, &start);
    int i, q_num = 0;
    int key;
    int scan_size = 100;
    for (i = 0; i < READ_NUM; i++)
    {
        key = keygen.Next();
        key = 0;
// scan_size=scansizegen.Next()+1;
#ifdef MULTITHREAD_SCAN
        for (q_num = 0; q_num < QUEUE_NUM; q_num++)
        {
            qs[q_num][tail[q_num]].seq = i;
            qs[q_num][tail[q_num]].key = key;
            qs[q_num][tail[q_num]].write = false;
            qs[q_num][tail[q_num]].scan_size = scan_size;
            tail[q_num]++;
        }
#else
        //global iterator init
        rocksdb::Iterator *iters[QUEUE_NUM];
        for (int j = 0; j < QUEUE_NUM; j++)
        {
            iters[j] = db[j]->NewIterator(rocksdb::ReadOptions(true, true));
        }
        MergingIterator *merge_iter = new MergingIterator(rocksdb::BytewiseComparator(), iters, QUEUE_NUM);
        char keystr[key_size_];
        GenerateKeyFromInt(key, keystr);
        merge_iter->Seek(rocksdb::Slice(keystr, key_size_));
        for (int j = 0; j < scan_size && merge_iter->Valid(); j++)
        {
            //  cout << merge_iter->key().ToString() << "  "<< unsigned(merge_iter->key().data()[6])<<","<<unsigned(merge_iter->key().data()[7])<<endl;
            merge_iter->Next();
        }
        delete merge_iter;
#endif
    }
    cout << i << endl;
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        qs[i][tail[i]].seq = -1;
        qs[i][tail[i]].key = -1;
        tail[i]++;
    }

    clock_gettime(CLOCK_REALTIME, &middle);
#ifdef MULTITHREAD_SCAN
    for (int j = 0; j < QUEUE_NUM; j++)
    {
        wts[j].join();
    }
    mt.join();
#endif
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t atime, btime;
    atime = duration_ns(start, middle);
    btime = duration_ns(start, end);
    cout << "generating requests:............." << endl;
    cout << "loading time per request (avg):" << atime / READ_NUM << "ns, QPS:" << 1000000000LL * READ_NUM / atime << ", throughputs:" << 1000000000LL * 128 / 1024 / 1024 * READ_NUM / atime << "MB/s" << endl;
    cout << "processing requests:............" << endl;
    cout << "request processing time (avg):" << btime / READ_NUM << "ns, QPS:" << 1000000000LL * READ_NUM / btime << ", throughputs:" << 1000000000LL * 128 / 1024 / 1024 * READ_NUM * 100 / btime << "MB/s" << endl;
    return 0;
}