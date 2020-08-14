# Benchmark Configuration
PLATFORM ?= fdw
SCENARIO ?= insert
DATASOURCE ?= lineitem.tbl
DATASIZE ?= 1  # GB
DBPATH ?= /tmp/vidardb_engine_benchmark

# PG Configuration
PGHOST ?= 127.0.0.1
PGPORT ?= 5432
PGDATABASE ?= postgres
PGUSER ?= postgres

# Build Configuration
CXX ?= c++
WCXXFLAGS = -Wno-deprecated-declarations
CXXFLAGS = -I/usr/local/include -I/usr/include -lpqxx -lpq -lvidardb $(WCXXFLAGS)
CXXFLAGS += ${EXTRA_CXXFLAGS} -O2

.PHONY: all
all: benchmark

.PHONY: benchmark
benchmark:
	$(CXX) src/benchmark.cc $(CXXFLAGS) -o benchmark

.PHONY: clean
clean:
	rm -rf benchmark

# install tpch
.PHONY: install-tpch
install-tpch:
	./benchmark.sh install_tpch

# generate tpch data
.PHONY: gen-data
gen-data:
	./benchmark.sh gen_data $(DATASIZE) $(DATASOURCE)

# run benchmark
.PHONY: run-benchmark
run-benchmark: clean all install-tpch
	PLATFORM=$(PLATFORM) SCENARIO=$(SCENARIO) DATASOURCE=$(DATASOURCE) \
	DATASIZE=$(DATASIZE) DBPATH=$(DBPATH) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
	PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) \
	./benchmark.sh run_benchmark $(PLATFORM)
