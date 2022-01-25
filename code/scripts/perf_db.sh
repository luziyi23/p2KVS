rm out.*
perf record -F 9999 -g -- ./db_bench --benchmarks=fillrandom --num=1000000 --value_size=100 --threads=8 --db=/mnt/optane-ssd/db --enable_pipelined_write=1 --allow_concurrent_memtable_write=false --batch_size=1 --compression_type=none
perf script > out.perf
../FlameGraph/stackcollapse-perf.pl out.perf > out.folded
../FlameGraph/flamegraph.pl out.folded > out.svg
sz out.svg
