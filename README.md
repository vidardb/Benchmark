# Benchmarking
This benchmark is to test insertion rate of PostgreSQL and the VidarDB (RocksDB style version).

1. Please don't use programming language such as Python to do the test, since a large percentage of time would be spent in language side. Use C++ provided.

2. We use TPC-H lineitem as the data source. You should first add a primary key (serial) to each record then shuffle the records. Becasue composite primary key is currently not supported in VidarDB, and the first attribute is implicitly as the primary key. We also don't assume data comes in primary key order.

3. It is best you can deploy two magnetic disks, one for read data source (usb3.0 is recommended), another for storing data. 

We have did test with TPC-H 10G and 1G in Ubuntu18.04 with PostgreSQL 11.6. You can follow the instruction in our repo of PostgreSQL Foreign Data Wrapper to enable VidarDB storage engine. 

Please don't hesitate to use issues. We will soon make the test more automatic.

