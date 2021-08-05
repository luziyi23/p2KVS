#1m测试
ulimit -n 65535
benchmark=SPLINTER  
tracemark=splinter  
# benchmark=YCSB
# tracemark=1M
# benchmark=LONGKEY 
# tracemark=longkey  
for loop3 in VALUE_128 #VALUE_64 VALUE_256 VALUE_512 VALUE_1024 VALUE_2048
do
    for loop in a #b c d e f
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
        for loop2 in Q8B32 #Q4B1 #Q4B32 Q1B32 Q8B1 Q4B1 Q1B1 #Q8B4 Q4B4 Q1B4
        do
            echo "$loop4 $loop $loop2"
            echo "g++ multibatch_hybrid.cpp -D ${benchmark}_LOAD -D ${loop2} -D ${loop3}"
            g++ multibatch_hybrid.cpp -m64 -D ${benchmark}_LOAD -D ${loop2} -D ${loop3} --std=c++14 -mcmodel=medium  -lrocksdb -lpthread -ldl -lzstd -llz4 -lbz2 -lsnappy -lz -o ycsb.out
            ./ycsb.out /mnt/optanessd/db /mnt/ssd/load_a_${tracemark}_0.trace
            echo "g++ multibatch_hybrid.cpp -D ${benchmark}_${workload} -D ${loop2} -D ${loop3}"
            g++ multibatch_hybrid.cpp -m64 -D ${benchmark}_${workload} -D ${loop2} -D ${loop3} --std=c++14 -mcmodel=medium  -lrocksdb -lpthread -ldl -lzstd -llz4 -lbz2 -lsnappy -lz -o ycsb.out
            sh flush.sh
            sleep 1
            ./ycsb.out /mnt/optanessd/db /mnt/ssd/run_${loop}_${tracemark}_0.trace
        done
        echo "----------------------------------------------------------------------------------"
    done
done