#1m测试
ulimit -n 65535
benchmark=SPLINTER  
# tracemark=splinter  
# benchmark=YCSB
# tracemark=1M
# benchmark=LONGKEY 
# tracemark=longkey
tracemark=ycsb  
sleep 5h
for loop in a b c d e f
do
    if [ $loop == e ]
    then
        workload=SCAN
    elif [ $loop == f ]
    then
        workload=F
    else 
    workload=RUN
    fi
    for loop2 in  Q4B1 Q8B1 #Q1B1 Q12B1
    do
        echo "$loop $loop2"
        echo "g++ multibatch_hybrid_rocksdb.cpp -D ${benchmark}_LOAD -D ${loop2}"
        g++ multibatch_hybrid_rocksdb.cpp -m64 -D ${benchmark}_LOAD -D ${loop2} --std=c++14 -mcmodel=medium  -lrocksdb -lpthread -ldl -lzstd -llz4 -lbz2 -lsnappy -lz -o ycsb.out
        ./ycsb.out /mnt/optanessd/db /mnt/ssd/trace/rocksdb/load_a_${tracemark}_${loop2}_
        echo "g++ multibatch_hybrid_rocksdb.cpp -D ${benchmark}_${workload} -D ${loop2}"
        g++ multibatch_hybrid_rocksdb.cpp -m64 -D ${benchmark}_${workload} -D ${loop2} --std=c++14 -mcmodel=medium  -lrocksdb -lpthread -ldl -lzstd -llz4 -lbz2 -lsnappy -lz -o ycsb.out
        sh flush.sh
        # sleep 1
        ./ycsb.out /mnt/optanessd/db /mnt/ssd/trace/run_${loop}_${tracemark}_${loop2}_
    done
    echo "----------------------------------------------------------------------------------"
done
