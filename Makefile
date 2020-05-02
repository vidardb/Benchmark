# Benchmark configuration
PLATFORM ?= fdw
DATASOURCE ?= lineitem.tbl
DATASIZE ?= 1  # GigaByte

PGHOST ?= 127.0.0.1
PGPORT ?= 5432
PGDATABASE ?= postgres
PGUSER ?= postgres

# Build configuration
CXX ?= c++
WOPTS = -Wno-deprecated-declarations
OPTS = -I/usr/local/include -I/usr/include -lpqxx -lpq $(WOPTS)

.PHONY: all
all: fdw_bench pg_bench

.PHONY: fdw_bench
fdw_bench:
	$(CXX) src/fdw_bench.cc $(OPTS) -o fdw_bench

.PHONY: pg_bench
pg_bench:
	$(CXX) src/pg_bench.cc $(OPTS) -o pg_bench

.PHONY: clean
clean:
	rm -rf fdw_bench pg_bench tpch-dbgen/lineitem.tbl

# install tpch
.PHONY: install-tpch
install-tpch:
	bash benchmark.sh install_tpch

# generate tpch data
.PHONY: gen-data
gen-data:
	bash benchmark.sh gen_data $(DATASIZE)

# run benchmark
.PHONY: run-benchmark
run-benchmark: clean all install-tpch
	DATASOURCE=$(DATASOURCE) DATASIZE=$(DATASIZE) \
	  PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
	  PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) \
	  bash benchmark.sh run_benchmark $(PLATFORM)
