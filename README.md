<img style="width:100%;" src="/github-banner.png">

# Benchmark

This benchmark is used to test the insert, load, scan of PostgreSQL and VidarDB (RocksDB style version).

We use TPC-H lineitem as the data source. Attention: ***The first attribute is implicit as the primary key for VidarDB engine.*** We also shuffle the data to avoid them coming in primary key order.

We have finished the testing with TPC-H 10G and 1G in Ubuntu 18.04 with PostgreSQL 11.6. You can follow the instruction in our repo of [PostgreSQL Foreign Data Wrapper](https://github.com/vidardb/PostgresForeignDataWrapper) to enable VidarDB storage engine. 

Please don't hesitate to [open an issue](https://github.com/vidardb/Benchmarking/issues) if you have a problem running with our benchmark.

## Building

1. Install [libpqxx](https://github.com/jtv/libpqxx) which is the official C++ client API for PostgreSQL and 6.x.x version is preferred.

    - For Ubuntu:

        ```shell
        sudo apt-get install -y libpqxx-dev
        ```

    - For Others:

        You can follow the [Building libpqxx](https://github.com/jtv/libpqxx#building-libpqxx) to install.

2. Install [VidarDB Engine](https://github.com/vidardb/vidardb#building) static library which is needed for the benchmark tools.

3. Build all benchmark tools in the root directory of the repo:

    ```shell
    make all
    ```

## Run benchmark

Before running benchmark, please ensure:

1. The PostgreSQL server has enabled [VidarDB Engine](https://github.com/vidardb/PostgresForeignDataWrapper).
2. The PostgreSQL server has no password and set trust in pg_hba.conf to 127.0.0.1/32 (currently the benchmark does not support password).
3. Best to clear the system cache (`sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"`) and restart the PostgreSQL server.


- Run benchmark scenario:

    ```shell
    # PLATFORM: benchmark database, optional values:
    #           pg: PostgreSQL
    #           fdw: VidarDB
    #           engine: VidarDB Engine
    #
    # SCENARIO: benchmark scenario, optional values:
    #           insert: line-by-line insertion
    #           load: for a bulk of lines
    #           scan: full scan
    #           getrand: get tuples by random primary keys  
    #           getlast: get tuples by last N primary keys 
    #
    # DATASIZE: benchmark data size, unit is GB.

    PLATFORM=fdw SCENARIO=insert DATASIZE=1 make run-benchmark
    ```
