# Benchmark configuration
DATASOURCE ?= scripts/tpch-dbgen/lineitem.tbl
DATASIZE ?= 1  # GigaByte

PGHOST ?= localhost
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
	rm -rf fdw_bench pg_bench

# install tpch
.PHONY: install-tpch
install-tpch:
	bash ./scripts/benchmark.sh install_tpch

# generate tpch data
.PHONY: gen-data
gen-data:
	bash ./scripts/benchmark.sh gen_data $(DATASIZE)

# TODO run benchmark
.PHONY: run-benchmark
run-benchmark: all
	@echo "\n==> Start to run PostgreSQL benchmark..."
	DATASOURCE=$(DATASOURCE) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
		PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) ./pg_bench

	@echo "\n==> Start to run VidarDB benchmark..."
	DATASOURCE=$(DATASOURCE) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
		PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) ./fdw_bench
