# Bottleneck analysis
In this experiment, we evaluate the write performance of RocksDB at different threads number and analyze the write latency breakdown by Linux-perf tool.

## Dependence

1. If you cannot use the `perf` command, install the Linux-perf tool  
```
sudo apt-get install linux-tools-common
sudo apt-get install linux-tools-"$(uname -r)"
sudo apt-get install linux-cloud-tools-"$(uname -r)"
sudo apt-get install linux-tools-generic
sudo apt-get install linux-cloud-tools-generic
```

2. For data visualization, we use the FlameGraph tool to show the latency breakdown. FlameGraph is already available as a submodule of this repository.

## Run testing

1. 

2. 