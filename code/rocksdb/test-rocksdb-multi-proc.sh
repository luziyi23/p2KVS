#!/bin/bash

dbname="/mnt/optanessd/db"
num=1250000
bytes_sync=$[1 * 1048576]  #1m
value_size=100
key_size=28
batch=1
direct=true
direct_read=false
nolog=true
for i in `seq 1 $1`
do
{
db=$dbname$i
echo $db
#rm -rf $db
#sync
#echo 3 > /proc/sys/vm/drop_caches
./db_bench --max_write_buffer_number=1000000 --max_num_range_tombstones=87500000 --wal_bytes_per_sync=$bytes_sync --db=$db --num=$num --use_existing_db=0  --compression_type=none --compression_ratio=1 --benchmarks="fillseq" --key_size=$key_size --value_size=$value_size --batch_size=$batch --use_direct_io_for_flush_and_compaction=$direct --use_direct_reads=$direct_read --disable_wal=$nolog  #--max_background_jobs=1 --level0_slowdown_writes_trigger=8 --level0_file_num_compaction_trigger=12
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --db=$db --num=$num --use_existing_db=0  --compression_type=none --compression_ratio=1 --benchmarks="fillrandom" --value_size=$value_size --batch_size=$batch --use_direct_io_for_flush_and_compaction=$direct --use_direct_reads=$direct_read --max_background_jobs=1
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --db=$dbname --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="overwrite" --value_size=$value_size
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --db=$db --num=$num  --use_direct_reads=true --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readseq" --value_size=$value_size
#sleep 5s
#sync
#echo 3 > /proc/sys/vm/drop_caches
#./db_bench --db=$db --num=$num --reads=100000 --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readrandom" --key_size=$key_size --value_size=$value_size
#./db_bench --threads=1 --use_direct_reads=false --db=$db --reads=200000 --seek_nexts=100 --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="seekrandom" --key_size=$key_size --value_size=$value_size


}&
done
wait
#sync


