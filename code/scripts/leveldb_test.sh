rm -rf /mnt/optane-ssd/ldb*
make clean
make write_test_leveldb INSTNUM=$1
make read_test_leveldb INSTNUM=$1
./write_test_leveldb /mnt/optane-ssd/ldb
sh ./scripts/flush_cache.sh
./read_test_leveldb /mnt/optane-ssd/ldb