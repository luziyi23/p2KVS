ROCKSDB_DIR = ./rocksdb
LEVELDB_DIR = ./leveldb
CXX = g++
CXXFLAGS = -O3 -std=c++14 -lpthread -mcmodel=medium
ROCKSDB_FLAG = -lrocksdb -ldl -lzstd -llz4 -lbz2 -lsnappy -lz
LEVELDB_FLAG = -lleveldb
OBJECTS = write_test read_test scan_test ycsb
.PHONY : clean rocksdb

all: install rocksdb


rocksdb: write_test read_test scan_test ycsb
	
	

# leveldb:
    # write_test_leveldb read_test_leveldb scan_test_leveldb ycsb_leveldb

install:
	$(MAKE) -C $(ROCKSDB_DIR) install
	# $(MAKE) -C $(LEVELDB_DIR)


write_test: write_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@

read_test: read_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ 

scan_test: scan_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ 

ycsb: ycsb.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ 

clean_rocksdb:
	$(MAKE) -C $(ROCKSDB_DIR) clean

clean:
	-rm $(OBJECTS)
