#ifndef PG_BENCHMARK_H_
#define PG_BENCHMARK_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <stdlib.h>
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/tablewriter>

#include "benchmark.h"
#include "util.h"

using namespace pqxx;

class PGBenchmarkScenario : public DBBenchmarkScenario {
  public:
    using DBBenchmarkScenario::BenchInsertScenario;
    virtual void BenchInsertScenario(void* args = nullptr) override;

    using DBBenchmarkScenario::PrepareBenchmarkData;
    virtual bool PrepareBenchmarkData() override;
};

void PGBenchmarkScenario::BenchInsertScenario(void* args) {
    work T(*C);
    tablewriter W(T, std::string(kTableName));
    std::ifstream in(std::string(getenv(kDataSource)));
    unsigned long long count = 0;

    std::cout << "Start to benchmark insertion rate ..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (std::string line; getline(in, line); ) {
        std::vector<std::string> row;
        for (int i = 0; i < 16; i++) {
            row.emplace_back(GetNthAttr(line, i));
        }
        W.insert(row);
        count++;
    }

    W.complete();
    T.commit();
    in.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    std::cout << "Insert " << count << " rows and take "
              << seconds << " s, tps = " << tps
              << std::endl;
}

bool PGBenchmarkScenario::PrepareBenchmarkData() {
    work T(*C);
    tablewriter W(T, std::string(kTableName));
    std::ifstream in(std::string(getenv(kDataSource)));

    std::cout << "Preparing benchmark data ..." << std::endl;
    for (std::string line; getline(in, line); ) {
        std::vector<std::string> row;
        for (int i = 0; i < 16; i++) {
            row.emplace_back(GetNthAttr(line, i));
        }
        W.insert(row);
    }

    W.complete();
    T.commit();
    in.close();
    return true;
}

BenchmarkScenario* NewPGBenchmarkScenario() {
    return new PGBenchmarkScenario();
}

#endif  /* PG_BENCHMARK_H_ */