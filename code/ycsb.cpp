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
#include "merge_iter.h"
#include "random.h"
#include "defines.h"

#define SEED 3090

#define MULTIGET
#define MULTISCAN

#define WRITE_BATCH_SIZE (BATCH_SIZE == 1 ? 1 : (4096 / (key_size_ + value_size_)))

using namespace std;

struct async_requests
{
    uint64_t key;
    int seq;
    int scan_size;
    bool write;
    async_requests(int s, int k) : seq(s), key(k)
    {
    }
    // async_requests(int s,int k,rocksdb::Slice v):seq(s),key(k),value(v){

    // }
    async_requests() : seq(0), key(0), scan_size(-1), write(false)
    {
    }
};

struct command
{
    uint64_t key;
    short scan_size;
    char cmd;
};

mutex mt;
async_requests qs[QUEUE_NUM][TOTAL_NUM / QUEUE_NUM + EXPAND_QUEUE_SIZE];
unsigned tail[QUEUE_NUM];
bool busy[QUEUE_NUM];
command cmds[TOTAL_NUM];

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

void worker_thread(int queue_seq, rocksdb::DB *db)
{
    rocksdb::Slice value;
    char *value_str = new char[value_size_];
    memset(value_str, '0', value_size_);
    value = rocksdb::Slice(value_str, value_size_);
    string readvalue;
    vector<string> values(BATCH_SIZE);
    timespec start, end;
    char key[key_size_];
    char scan_end_key[key_size_];
    rocksdb::Slice k(key, key_size_);
    rocksdb::Slice scan_end(scan_end_key, key_size_);
    int seq = 0;
    bool endflag = 0;
    int batch_size = 0;
    std::vector<rocksdb::Slice> mutliget_keys;
    rocksdb::WriteBatch batch;
    auto writeoptions = rocksdb::WriteOptions();
    // writeoptions.disableWAL=true;
    auto readoptions = rocksdb::ReadOptions(true, true);
    const rocksdb::Comparator *comparator = rocksdb::BytewiseComparator();
    rocksdb::Status s;
    vector<rocksdb::Status> ss;
    unsigned long long found = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    while (!endflag)
    {
        if (seq >= tail[queue_seq])
        {
            if (batch_size)
            {
                s = db->Write(writeoptions, &batch);
                if (s.ok())
                    found += batch_size;
                batch.Clear();
                batch_size = 0;
            }
#ifdef MULTIGET
            if (mutliget_keys.size())
            {
                values.clear();
                ss.clear();
                ss = db->MultiGet(readoptions, mutliget_keys, &values);
                for (auto a : ss)
                {
                    if (a.ok())
                        found++;
                }
                mutliget_keys.clear();
            }
#endif
            busy[queue_seq] = false;
            while (seq >= tail[queue_seq])
            {
                this_thread::sleep_for(chrono::microseconds(2000));
            }
            busy[queue_seq] = true;
        }
        async_requests request = qs[queue_seq][seq];

        if (request.seq == -1)
        {
            if (batch_size)
            {
                s = db->Write(writeoptions, &batch);
                if (s.ok())
                    found += batch_size;
            }
#ifdef MULTIGET
            if (mutliget_keys.size())
            {
                ss = db->MultiGet(readoptions, mutliget_keys, &values);
                for (auto a : ss)
                {
                    if (a.ok())
                        found++;
                }
            }
#endif
            endflag = true;
            break;
        }
        seq++;
        if (request.write)
        {
            GenerateKeyFromInt(request.key, key);
            batch.Put(k, value);
            batch_size++;
            if (batch_size >= WRITE_BATCH_SIZE)
            {
                s = db->Write(writeoptions, &batch);
                if (s.ok())
                {
                    found += batch_size;
                }
                else
                {
                    mt.lock();
                    cout << s.ToString() << endl;
                    mt.unlock();
                }
                batch.Clear();
                batch_size = 0;
            }
        }
        else 
        {
            GenerateKeyFromInt(request.key, key);
            if (batch_size)
            {
                s = db->Write(writeoptions, &batch);
                if (s.ok())
                    found += batch_size;
                else
                {
                    mt.lock();
                    cout << s.ToString() << endl;
                    mt.unlock();
                }
                batch.Clear();
                batch_size = 0;
                // }
            }
            if (request.scan_size == -1)
            { //point query
#ifdef MULTIGET
                mutliget_keys.push_back(rocksdb::Slice(key, key_size_));
                if (mutliget_keys.size() == BATCH_SIZE)
                {
                    //multiget
                    values.clear();
                    ss.clear();
                    ss = db->MultiGet(readoptions, mutliget_keys, &values);
                    for (auto a : ss)
                    {
                        if (a.ok())
                            found++;
                        else
                        {
                            mt.lock();
                            cout << a.ToString() << endl;
                            mt.unlock();
                        }
                    }
                    mutliget_keys.clear();
                }
#else
                s = db->Get(readoptions, rocksdb::Slice(key, key_size_), &readvalue);
                if (s.ok())
                {
                    found++;
                }
                else
                {
                    mt.lock();
                    cout << queue_seq << "=" << request.key % QUEUE_NUM << " " << request.seq << "," << seq << "" << s.ToString() << request.key << endl;
                    mt.unlock();
                }

#endif
            }
            else
            { //scan
                // GenerateKeyFromInt(request.key+request.scan_size,scan_end_key);
                Iterator *iter = db->NewIterator(readoptions);
                iter->Seek(k);
                found++;
                for (int i = 1; i < request.scan_size && iter->Valid(); i++)
                {
                    found++;
                    iter->Next();
                }
                delete iter;
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    auto atime = duration_ns(start, end);
    string stats;
    db->GetProperty("rocksdb.levelstats", &stats);
    mt.lock();
    cout << "thread " << queue_seq << " has processed " << found << "/" << seq << "requests. Avg latency:" << atime / seq << "ns, QPS=" << 1000000000LL * seq / atime << ", throughputs=" << 1000000000LL * (key_size_ + value_size_) / 1024 / 1024 * found / atime << "MB/s" << endl;
    cout << stats << endl;
    mt.unlock();
}
int main(int argc, char *argv[])
{
    bool destroydb = false;
    if (argc != 3)
    {
        printf("please enter the directory of db_path and trace_file as the two paramaters \n(e.g.: ycsb /mnt/optanessd/db trace/load_a_0.trace)\n");
        return 0;
    }
#ifndef TOTAL_NUM
    quit(-1);
#endif
    char filename[50] = "trace/load_b_1M_0.trace";
    strcpy(filename, argv[2]);
    char db_path[50] = "/mnt/optanessd/db_ycsb_thread";
    strcpy(db_path, argv[1]);
    if (strstr(filename, "load") != NULL)
        destroydb = true;
    cout << filename << endl;

    //db initialize
    const string DBPath = db_path;
    rocksdb::DB *db[QUEUE_NUM];
    rocksdb::Options options;
    options.compression = rocksdb::kNoCompression;
    options.create_if_missing = true;
    options.use_direct_io_for_flush_and_compaction = true;
    // options.max_file_opening_threads=64;
    // options.use_direct_reads=true;
    options.wal_bytes_per_sync = 4 * 1048576;
    options.max_background_jobs = 44 - QUEUE_NUM;
    rocksdb::BlockBasedTableOptions *table_options =
        reinterpret_cast<rocksdb::BlockBasedTableOptions *>(options.table_factory->GetOptions());
    table_options->filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        char c[16];
        std::sprintf(c, "%03d", i);
        if (destroydb)
        {
            rocksdb::DestroyDB(DBPath + c, options);
        }
        rocksdb::Status status = rocksdb::DB::Open(options, DBPath + c, &db[i]);
        assert(status.ok());
    }
    rocksdb::Random64 *rand = new rocksdb::Random64(SEED);
    KeyGenerator keygen(rand, WriteMode::RANDOM, TOTAL_NUM);


    // read requests from trace file into memory
    FILE *fp;
    fp = fopen(filename, "r");
    assert(fp != NULL);
    uint64_t key;
    char valuebuf[1024];
    int valuelen;
    size_t linesize = 50;
    int line_num = 0;
    int total_scan = 0;
    int ret;
    char *buf = (char *)malloc(50);
    cout << "write batch_size= " << WRITE_BATCH_SIZE << endl;
    while (1)
    {
        ret = getline(&buf, &linesize, fp);
        if (ret <= 0)
            break;
        cmds[line_num].cmd = buf[0];
        char *keybuf = buf + 6;
        key = atol(keybuf);
        cmds[line_num].key = key;
        if (buf[0] == 'S')
        {
            char *scansizebuf = strchr(keybuf, ' ') + 1;
            cmds[line_num].scan_size = atoi(scansizebuf);
        }
        line_num++;
    }
    fclose(fp);
    cout << line_num << endl;
    // initialize request processing threads
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        tail[i] = 0;
    }
    thread wts[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        wts[i] = thread(worker_thread, i, db[i]);
    }
    timespec start, middle, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // issue user requests
    char cmd[10];
    int scan_size;
    int ops = 0;
    int i = 0;
    int S = 0;
    command *c;
    unsigned q_num;
    while (i < line_num)
    {
        c = &cmds[i];
        i++;
        if (c->cmd == 'R')
        {
            key = c->key;
            q_num = key % (unsigned)QUEUE_NUM;
            qs[q_num][tail[q_num]].seq = ops;
            qs[q_num][tail[q_num]].key = key;
            qs[q_num][tail[q_num]].write = false;
            tail[q_num]++;
        }
        else if (c->cmd == 'U' || c->cmd == 'I')
        {
            key = c->key;
            q_num = key % (unsigned)QUEUE_NUM;
            qs[q_num][tail[q_num]].seq = ops;
            qs[q_num][tail[q_num]].key = key;
            qs[q_num][tail[q_num]].write = true;
            tail[q_num]++;
        }
        else if (c->cmd == 'D')
        {
            cout << "aaaa" << endl;
        }
        else if (c->cmd == 'S')
        {
            S++;
            total_scan += c->scan_size;
#ifdef MULTISCAN
// parallel scan
            for (int i = 0; i < QUEUE_NUM; i++)
            {
                qs[i][tail[i]].seq = ops;
                qs[i][tail[i]].key = c->key;
                qs[i][tail[i]].scan_size = c->scan_size;
                qs[i][tail[i]].write = false;
                tail[i]++;
            }
#else
// single thread scan
            //wait for previous reqeusts finishing
            for (int j = 0; j < QUEUE_NUM; j++)
            {
                while (busy[j])
                {
                    this_thread::sleep_for(chrono::microseconds(1));
                }
            }
            //iterator initialize
            rocksdb::Iterator *iters[QUEUE_NUM];
            for (int j = 0; j < QUEUE_NUM; j++)
            {
                iters[j] = db[j]->NewIterator(rocksdb::ReadOptions(true, true));
            }
            MergingIterator *merge_iter = new MergingIterator(rocksdb::BytewiseComparator(), iters, QUEUE_NUM);
            char keystr[key_size_];
            GenerateKeyFromInt(c->key, keystr);
            merge_iter->Seek(rocksdb::Slice(keystr, key_size_));
            for (int j = 0; j < c->scan_size && merge_iter->Valid(); j++)
            {
                ops++;
                merge_iter->Next();
            }
            // for(int j=0;j<QUEUE_NUM;j++){
            //     delete iters[j];
            // }
            delete merge_iter;
#endif
        }
        ops++;
    }
    cout << S << ":" << total_scan << endl;
    for (int i = 0; i < QUEUE_NUM; i++)
    {
        // cout <<i<<":"<< tail[i] << endl;
        qs[i][tail[i]].seq = -1;
        qs[i][tail[i]].key = -1;
        tail[i]++;
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
    cout << i << " " << ops << " " << total_scan << endl;
    cout << "loading trace file:............." << endl;
    cout << "loading time per request (avg):" << atime / i << "ns, QPS:" << 1000000000LL * i / atime << ", throughputs: " << 1000000000LL * (key_size_ + value_size_) / 1024 / 1024 * ops / atime << "MB/s" << endl;
    cout << "processing requests:............" << endl;
    cout << "request processing time (avg):" << btime / i << "ns, QPS:" << 1000000000LL * i / btime << ", throughputs: " << 1000000000LL * (key_size_ + value_size_) / 1024 / 1024 * ops / btime << "MB/s" << endl
         << endl;
    if (destroydb)
    {
        for (int j = 0; j < QUEUE_NUM; j++)
        {
            db[j]->Close();
        }
    }
    return 0;
}