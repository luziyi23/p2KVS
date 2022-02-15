rm -rf /mnt/optane-ssd/wt00*
make clean
make write_test_wt INSTNUM=$1
make read_test_wt INSTNUM=$1
./write_test_wt /mnt/optane-ssd/wt
sh ./scripts/flush_cache.sh
./read_test_wt /mnt/optane-ssd/wt