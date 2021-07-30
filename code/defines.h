#ifdef Q1B1
    #define QUEUE_NUM 1
    #define BATCH_SIZE 1
#endif
#ifdef Q4B1
    #define QUEUE_NUM 4
    #define BATCH_SIZE 1
#endif
#ifdef Q8B1
    #define QUEUE_NUM 8
    #define BATCH_SIZE 1
#endif
#ifdef Q12B1
    #define QUEUE_NUM 12
    #define BATCH_SIZE 1
#endif
#ifdef Q1B16
    #define QUEUE_NUM 1
    #define BATCH_SIZE 16
#endif
#ifdef Q4B16
    #define QUEUE_NUM 4
    #define BATCH_SIZE 16
#endif
#ifdef Q8B16
    #define QUEUE_NUM 8
    #define BATCH_SIZE 16
#endif
#ifdef Q1B32
    #define QUEUE_NUM 1
    #define BATCH_SIZE 32
#endif
#ifdef Q4B32
    #define QUEUE_NUM 4
    #define BATCH_SIZE 32
#endif
#ifdef Q8B32
    #define QUEUE_NUM 8
    #define BATCH_SIZE 32
#endif
#ifdef Q1B4
    #define QUEUE_NUM 1
    #define BATCH_SIZE 4
#endif
#ifdef Q4B4
    #define QUEUE_NUM 4
    #define BATCH_SIZE 4
#endif
#ifdef Q8B4
    #define QUEUE_NUM 8
    #define BATCH_SIZE 4
#endif
#ifdef Q12B32
    #define QUEUE_NUM 12
    #define BATCH_SIZE 32
#endif
#ifndef QUEUE_NUM
    #define QUEUE_NUM 1
    #define BATCH_SIZE 1
#endif
#ifdef SPLINTER_LOAD
    #define TOTAL_NUM 673000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/100+1
    #define key_size_ 24
    #define value_size_ 100
#endif

#ifdef SPLINTER_SCAN
    #define TOTAL_NUM 20000000LL
    #define EXPAND_QUEUE_SIZE (TOTAL_NUM*(QUEUE_NUM-1)/QUEUE_NUM)
    #define key_size_ 24
    #define value_size_ 100
#endif
#ifdef SPLINTER_F
    #define TOTAL_NUM 240000805LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/QUEUE_NUM
    #define key_size_ 24
    #define value_size_ 100
#endif
#ifdef SPLINTER_RUN
    #define TOTAL_NUM 160000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 24
    #define value_size_ 100
#endif
#ifdef YCSB_LOAD
    #define TOTAL_NUM 1000000
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef YCSB_RUN
    #define TOTAL_NUM 1000000
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef YCSB_SCAN
    #define TOTAL_NUM 1000000
    #define EXPAND_QUEUE_SIZE TOTAL_NUM
    #define key_size_ 28
    #define value_size_ 100
#endif
#ifdef YCSB_F
    #define TOTAL_NUM 1500000
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    #define value_size_ 100
#endif
#ifdef LONGKEY_LOAD
    #define TOTAL_NUM 100000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef LONGKEY_RUN
    #define TOTAL_NUM 20000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef LONGKEY2_LOAD
    #define TOTAL_NUM 100000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef LONGKEY2_RUN
    #define TOTAL_NUM 20000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef LONGKEY_SCAN
    #define TOTAL_NUM 3000000LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef LONGKEY_F
    #define TOTAL_NUM 29998597LL
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    // #define value_size_ 100
#endif
#ifdef VALUE_64
    #define value_size_ 36
#endif
#ifdef VALUE_128
    #define value_size_ 100
#endif
#ifdef VALUE_256
    #define value_size_ 228
#endif
#ifdef VALUE_512
    #define value_size_ 484
#endif
#ifdef VALUE_1024
    #define value_size_ 998
#endif
#ifdef VALUE_2048
    #define value_size_ 2020
#endif
#ifdef VALUE_4096
    #define value_size_ 4068
#endif

#ifndef TOTAL_NUM
    #define TOTAL_NUM 1000000
    #define EXPAND_QUEUE_SIZE TOTAL_NUM/10+1
    #define key_size_ 28
    #define value_size_ 100
#endif

