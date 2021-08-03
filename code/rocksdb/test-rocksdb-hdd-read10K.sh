#!/bin/bash

dbname="/mnt/hdd/db"
num=10000000
read=100000
value_size=100

rm -rf $dbname
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --db=$dbname --num=$num --use_existing_db=0  --compression_type=none --compression_ratio=1 --benchmarks="fillseq" --value_size=$value_size
sleep 5s
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --db=$dbname --num=$num --use_existing_db=0  --compression_type=none --compression_ratio=1 --benchmarks="fillrandom" --value_size=$value_size
sleep 5s
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --db=$dbname --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="overwrite" --value_size=$value_size
sleep 5s
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --db=$dbname --num=$num --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readseq" --value_size=$value_size
sleep 5s
sync
echo 3 > /proc/sys/vm/drop_caches
./db_bench --db=$dbname --num=$num --reads=$read --use_existing_db=1  --compression_type=none --compression_ratio=1 --benchmarks="readrandom" --value_size=$value_size 



