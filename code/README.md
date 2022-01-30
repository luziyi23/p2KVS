This repository only exposes part of the test scripts and related codes, mainly including core micro- and marco-benchmark tests of p2KVS.

# Dependences
In the sections of motivation and evaluation in our paper, we used some tools for analysis and comparison, so we introduced them as submodules. To execute these parts of the code, please install the submodules.  

   ```
   git submodule update --init --recursive
   ```

## RocksDB
We made some non-invasive changes to the Rocksdb code and integrated its compile installation into the Makefile. Note that the original RocksDB is still available in p2KVS. 
   ```
   make install
   ```
## LevelDB
We did not modify the LevelDB code, so LevelDB is integrated into the project as a submodule.
   ```
   cd leveldb
   cmake .
   make install
   ```


# Bottleneck analysis
Check bottleneck_analysis.md.

# Evaluate p2KVS with micro-benchmarks and YCSB
Check evaluation.md.

