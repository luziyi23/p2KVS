#!/bin/bash
ulimit -i 65536
dbname="/mnt/optanessd/db"
num=100000000
key_size=28
value_size=1000 #$[1 * 1048576]
batch=1
bytes_sync=$[4 * 1048576]  #1m
nolog=false
columns=1
#rm -rf $dbname
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --threads=1 --max_num_range_tombstones=0 --sync=false --use_direct_io_for_flush_and_compaction=true --num_column_families=$columns  --db=$dbname  --num=$num --use_existing_db=0  --compression_type=none --compression_ratio=1 --benchmarks="filluniquerandom" --stats_interval_seconds=1 --key_size=$key_size --value_size=$value_size --batch_size=$batch --disable_wal=$nolog --wal_bytes_per_sync=$bytes_sync  --enable_pipelined_write=false --allow_concurrent_memtable_write=false --max_background_jobs=4 #-max_write_buffer_number=100 #--write_buffer_size=4194304 --target_file_size_base=2097152 --level0_slowdown_writes_trigger=8 --level0_stop_writes_trigger=12 --max_background_jobs=16
#sleep 5s
#rm -rf $dbname
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --num_column_families=$columns --db=$dbname  --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="fillrandom" --key_size=$key_size --value_size=$value_size --batch_size=$batch --disable_wal=$nolog --enable_pipelined_write=false --wal_bytes_per_sync=$bytes_sync --allow_concurrent_memtable_write=false --bloom_bits=10 #--write_buffer_size=4194304 --target_file_size_base=2097152 --level0_slowdown_writes_trigger=8 --level0_stop_writes_trigger=12 
#--max_background_jobs=64 --num_high_pri_threads=32 --num_low_pri_threads=32


#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --threads=1 --use_direct_reads=false --db=$dbname --reads=200000 --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readseq" --key_size=$key_size --value_size=$value_size
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --threads=2 --db=$dbname --num=$num --batch_size=$batch --reads=32000 --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readrandom" --key_size=$key_size --value_size=$value_size --bloom_bits=10
#./db_bench  --threads=1 --db=$dbname --num=$num --batch_size=32 --reads=100000 --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="multireadrandom" --key_size=$key_size --value_size=$value_size # --bloom_bits=10
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --db=$dbname --num=1000000 --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="overwrite" --key_size=$key_size --value_size=$value_size --bloom_bits=10 



