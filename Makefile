# Benchmark configuration
PLATFORM ?= fdw
DATASOURCE ?= lineitem.tbl
DATASIZE ?= 1  # GB
DBPATH ?= /tmp/vidardb_engine_benchmark

# PG configuration
PGHOST ?= 127.0.0.1
PGPORT ?= 5432
PGDATABASE ?= postgres
PGUSER ?= postgres

# Build configuration
CXX ?= c++
WCXXFLAGS = -Wno-deprecated-declarations
CXXFLAGS = -I/usr/local/include -I/usr/include -lpqxx -lpq -lvidardb $(WCXXFLAGS)
CXXFLAGS += ${EXTRA_CXXFLAGS}

.PHONY: all
all: fdw_bench pg_bench engine_bench

.PHONY: fdw_bench
fdw_bench:
	$(CXX) src/fdw_bench.cc $(CXXFLAGS) -o fdw_bench

.PHONY: pg_bench
pg_bench:
	$(CXX) src/pg_bench.cc $(CXXFLAGS) -o pg_bench

.PHONY: engine_bench
engine_bench:
	$(CXX) src/engine_bench.cc $(CXXFLAGS) -o engine_bench

.PHONY: clean
clean:
	rm -rf fdw_bench pg_bench engine_bench

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
	DATASOURCE=$(DATASOURCE) DATASIZE=$(DATASIZE) \
	  DBPATH=$(DBPATH) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
	  PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) \
	  ./benchmark.sh run_benchmark $(PLATFORM)
