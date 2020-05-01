# benchmark configuration
DATASOURCE ?= scripts/tpch-dbgen/lineitem.tbl
DATASIZE ?= 1  # G

PGHOST ?= localhost
PGPORT ?= 5432
PGDATABASE ?= postgres
PGUSER ?= postgres

# build configuration
CXX ?= g++
OPTS = "-fPIC -std=c++11 -Wno-deprecated-declarations"

default: all

.PHONY: all
all: clean fdw_bench psql_bench

.PHONY: fdw_bench
fdw_bench:
	$(CXX) src/fdw_benchmark.cc $(OPTS) -o fdw_bench

.PHONY: psql_bench
psql_bench:
	$(CXX) src/psql_benchmark.cc $(OPTS) -o psql_bench

.PHONY: clean
clean:
	rm -rf fdw_bench psql_bench

# install tpch
.PHONY: install-tpch
install-tpch:
	bash ./scripts/benchmark.sh install_tpch

# generate tpch data
.PHONY: gen-data
gen-data:
	bash ./scripts/benchmark.sh gen_data $(DATASIZE)

# run benchmark
.PHONY: bench
bench: all
	@echo "\n==> start to run PostgreSQL benchmark..."
	DATASOURCE=$(DATASOURCE) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
		PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) ./psql_bench

	@echo "\n==> start to run VidarDB benchmark..."
	DATASOURCE=$(DATASOURCE) PGHOST=$(PGHOST) PGPORT=$(PGPORT) \
		PGDATABASE=$(PGDATABASE) PGUSER=$(PGUSER) ./fdw_bench
