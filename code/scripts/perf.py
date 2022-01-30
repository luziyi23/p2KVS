import os
import time

total_num = 10000000
thread_nums = [1, 2, 4, 8, 12, 16, 24, 32]
for threads in thread_nums:
    num = int(total_num / threads)
    tombstone = total_num - num
    cmd = "perf record -F 9999 -g -- ../rocksdb/db_bench --benchmarks=fillrandom,stats --num=%(num)s --max_num_range_tombstones=%(" \
          "tombstone)s --threads=%(threads)s  --db=/mnt/optane-ssd/db --compression_ratio=1 --compression_type=none >> test_results.txt" % {"num": num,
                                                                                                  "threads": threads,
                                                                                                  "tombstone": tombstone}
    os.system(cmd)
    os.system("perf script > out.perf")
    os.system("../../../FlameGraph/stackcollapse-perf.pl out.perf > out.folded")
    os.system("../../../FlameGraph/flamegraph.pl out.folded > perf_%s.svg" % (threads))
    os.system('sync')
    time.sleep(5)
