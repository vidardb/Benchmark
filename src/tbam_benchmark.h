#ifndef TBAM_BENCHMARK_H_
#define TBAM_BENCHMARK_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
using namespace std;

#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/tablewriter>
using namespace pqxx;

#include "benchmark.h"
#include "util.h"

class TBAMBenchmarkScenario : public PGBenchmarkScenario {
};

BenchmarkScenario* NewTBAMBenchmarkScenario() {
    return new TBAMBenchmarkScenario();
}

#endif  /* TBAM_BENCHMARK_H_ */