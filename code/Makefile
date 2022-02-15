ROCKSDB_DIR = ./rocksdb
LEVELDB_DIR = ./leveldb
CXX = g++
CXXFLAGS = -O3 -std=c++14 -lpthread -mcmodel=medium
ROCKSDB_FLAG = -lrocksdb -ldl -lzstd -llz4 -lbz2 -lsnappy -lz
LEVELDB_FLAG = -lleveldb
OBJECTS = write_test read_test scan_test ycsb write_test_wt write_test_wt2 read_test_wt read_test_leveldb write_test_leveldb

DATANUM?=10000000
INSTNUM?=8
READNUM?=1000000
TOTAL_NUM=${DATANUM}
QUEUE_NUM=${INSTNUM}
READ_NUM=${READNUM}


.PHONY : clean rocksdb

all: install rocksdb


rocksdb: write_test read_test scan_test ycsb
	

leveldb: write_test_leveldb read_test_leveldb 

install:
	$(MAKE) -C $(ROCKSDB_DIR) db_bench install

write_test_default: write_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o write_test

write_test: write_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)

read_test_default: read_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o read_test

read_test: read_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM) -DREAD_NUM=${READ_NUM}

scan_test_default: scan_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o scan_test

scan_test: scan_test.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)

ycsb: ycsb.cpp
	$(CXX) $^ $(CXXFLAGS) $(ROCKSDB_FLAG) -o $@ -D${BENCHMARK} -D${IB} -D${KVSIZE}


write_test_leveldb: write_test_leveldb.cpp
	g++ $^  -mcmodel=medium --std=c++14 -lleveldb -lpthread -lsnappy -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)

read_test_leveldb: read_test_leveldb.cpp
	g++ $^  -mcmodel=medium --std=c++14 -lleveldb -lpthread -lsnappy -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)  -DREAD_NUM=${READ_NUM}

write_test_wt: write_test_wt.cpp
	g++ $^ --std=c++14 -lpthread -lwiredtiger -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)  -DREAD_NUM=${READ_NUM}

write_test_wt2: write_test_wiredtiger.cpp
	g++ $^ --std=c++14 -lpthread -lwiredtiger -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)  -DREAD_NUM=${READ_NUM}

read_test_wt: read_test_wt.cpp
	g++ $^ --std=c++14 -lpthread -lwiredtiger -o $@ -DTOTAL_NUM=$(TOTAL_NUM) -DQUEUE_NUM=$(QUEUE_NUM)  -DREAD_NUM=${READ_NUM}

clean_rocksdb:
	$(MAKE) -C $(ROCKSDB_DIR) clean

clean:
	-rm -f $(OBJECTS)
