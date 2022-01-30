# Evaluate p2KVS
We use YCSB benchmarks to generate trace files. The p2KVS prototype can load the trace files and process the operations in the traces. Note that the codes of YCSB has been modified to generate  trace files that conform to a certain format.

Here is the process for generating trace using the modified YCSB:
1. Build ycsb

   ```sh
   mvn -pl site.ycsb:core -am package -DskipTests dependency:build-classpath -DincludeScope=compile -Dmdep.outputFilterFile=true
   ```

2. Generate trace file with certain workload properties. The properties can be changed by modifying the configurations file in workloads directory.
   ```sh
   bin/ycsb load basic -P workloads/workloada -trace loada -p "fieldcount=1"
   #workloada is the configuration file of trace properties.
   # loada is the prefix of trace file name.
   ```
3. Generate traces of a multi-threaded workloads by adding the thread parameter (used to test the performance of a single instance under multiple threads).
   ```sh
    bin/ycsb load basic -P workloads/workloada -trace loada -p "fieldcount=1" -p "threadcount=4"
   ```

# Testing RocksDB with p2KVS
We made scripts for four test cases, including write tests, read tests, scan tests, and YCSB tests. The first three tests generate a fixed number of requests that are then executed on the Rocksdb instance via p2KVS. The YCSB test loads the request recorded in the trace file into memory and then executes on p2KVS. 

Before performing the read and scan tests, you need to load a certain number of datasets through the write test. Each YCSB test are often done in two steps by running the trace that loads the dataset and the trace that actually executes workloads.

The number of instances (INSTNUM) in p2KVS and the size of dataset (DATANUM) can be set at compile time. The environment variable DATANUM represents the size of the data set, the number of records that will be written in the write test. In read and scan tests, the amount of data to be read (READNUM) need to be specified.

## microbenchmark
1. Complie p2KVS prototype test program in the default settings.
    
    compire:
   ```sh
   make write_test_default
   make read_test_default
   make scan_test_default
   ```
   run(use the DB directory as the argument):
   ```sh
   write_test /mnt/sdd/db
   read_test /mnt/sdd/db
   scan_test /mnt/sdd/db
   ```
2. Complie p2KVS with custom numbers of requests and instance.
   ```
   make write_test DATANUM=32000000 INSTNUM=8
   make read_test DATANUM=32000000 READNUM=1000000 INSTNUM=8
   make scan_test DATANUM=32000000 READNUM=10000 INSTNUM=8
   ```
   Note that the number of instances when testing the same DB must be the same (the number of requests can vary).

## ycsb
We have recorded the configurations used in our YCSB experiments in the file "defines.h", which can be invoked using definitions.
Here is the explanations of the definitions.

### Benchmarks. 

The macro is in format {NAME}_{BENCHMARK} (e.g. YCSB_RUN, SPLINTER_LOAD).

The NAME parameter includes YCSB and SPLINTER. YCSB represents a small test containing a load data set of 1,000,000 write operations and ycsb run tests of 1,000,000 operations.
SPLINTER represents the same configuration as splinterDB (ATC '20), including including a dataset of 673M records, 160M requests for each YCSB test, except for 20M requests for the YCSBE test. 

The BENCHMARK parameter includes LOAD, RUN, SCAN, and F. LOAD means loading the dataset. RUN means ycsb a~d. SCAN means ycsb e and F means ycsb f.

### Instance number and max batching size
The macro is in format Q{N}B{M} (e.g. Q4B32 Q8B1).

N represents the number of instances, and the candidates are 1, 4, 8 and 12. M represents the maximum request batching size, and the candidates are 1 and 32, representing the on and off of OBM mechanism, respectively.

### Value size
This is an optional parameter in the format of VALUE_{S} (e.g. VALUE_128). This parameter is used to modify the size of the key-value pair in bytes. The default key-value pair size is 128 bytes.


After the YCSB test program has been compiled with the appropriate configuration selected from the macro definition above, execute the YCSB test by the pre-generated trace file:
```sh
# complie 
make ycsb BENCHMARK={NAME}_LOAD IB=Q8B32 KVSIZE=VALUE_128
#run ycsb load
ycsb /mnt/optanessd/db trace/load_a_0.trace
# complie
make ycsb BENCHMARK={NAME}_RUN IB=Q8B32 KVSIZE=VALUE_128
#run a~d
ycsb /mnt/optanessd/db trace/run_a_0.trace
ycsb /mnt/optanessd/db trace/run_b_0.trace
ycsb /mnt/optanessd/db trace/run_c_0.trace
ycsb /mnt/optanessd/db trace/run_d_0.trace
# complie 
make ycsb BENCHMARK={NAME}_SCAN IB=Q8B32 KVSIZE=VALUE_128
# run ycsb e
ycsb /mnt/optanessd/db trace/run_e_0.trace
# complie
make ycsb BENCHMARK={NAME}_F IB=Q8B32 KVSIZE=VALUE_128
# run ycsb f
ycsb /mnt/optanessd/db trace/run_f_0.trace
```

It is recommended to refer to scripts/ycsb.sh to perform a complete YCSB test. 
