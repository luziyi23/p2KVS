# Bottleneck analysis
In this experiment, we evaluate the write performance of RocksDB at different threads number and analyze the write latency breakdown by Linux-perf tool.

## Dependence
1. Verify that RocksDB `db_bench` has been installed. See README.md for installation methods.  
2. If you cannot use the `perf` command, install the Linux-perf tool
```
sudo apt-get install linux-tools-common
sudo apt-get install linux-tools-"$(uname -r)"
sudo apt-get install linux-cloud-tools-"$(uname -r)"
sudo apt-get install linux-tools-generic
sudo apt-get install linux-cloud-tools-generic
```

3. For data visualization, we use the FlameGraph tool to show the latency breakdown. FlameGraph is already available as a submodule of this repository.

4. Python3 is needed to run the scripts.

## Run testing
1. Input the SSD directory
```
cd scripts
vim perf.py
# Assign db_dir with an SSD directory
```

2. Run test script
```
python perf.py
```

3. See throuputs and latencies in `test_result.txt`.
See flame graphs of latency breakdown in `perf_[#threads].svg`.