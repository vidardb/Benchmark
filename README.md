# Benchmarking

This benchmark is used to test the insertion rate of PostgreSQL and the VidarDB (RocksDB style version).

1. Please don't use "slow" programming language such as Python to do the test, since a large percentage of time would be spent in language side. Use C++ instead.

2. We use TPC-H lineitem as the data source. You should first add a primary key (serial) to each record then shuffle the records.  Attention: ***composite primary key is currently not supported by VidarDB, and the first attribute is implicit as the primary key.*** We also don't assume data comes in primary key order.

3. You can deploy two magnetic disks, one for reading data source (usb3.0 is recommended), another for storing data. 

We have finished the testing with TPC-H 10G and 1G in Ubuntu 18.04 with PostgreSQL 11.6. You can follow the instruction in our repo of [PostgreSQL Foreign Data Wrapper](https://github.com/vidardb/PostgresForeignDataWrapper) to enable VidarDB storage engine. 

Please don't hesitate to [open an issue](https://github.com/vidardb/Benchmarking/issues) if you have a problem running with our benchmark.

We plan to make the test more automatic soon.

## Building

1. Install [libpgxx-6.4.6](https://github.com/jtv/libpqxx) which is the official C++ client API for PostgreSQL.

2. Build all benchmark tools in the root directory of the repo:

    ```shell
    make all
    ```

## Run benchmark

*Beforce running benchmark, please ensure the PostgreSQL server which has enabled [VidarDB engine](https://github.com/vidardb/PostgresForeignDataWrapper) is running in your local machine.*

- Run PostgreSQL benchmark:

```shell
# DATASIZE: benchmark data size, unit is GB.

PLATFORM=pg DATASIZE=1 make run-benchmark
```

- Run VidarDB benchmark:

```shell
# DATASIZE: benchmark data size, unit is GB.

PLATFORM=fdw DATASIZE=1 make run-benchmark
```