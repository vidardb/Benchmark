#ifndef PG_BENCHMARK_H_
#define PG_BENCHMARK_H_

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

class PGBenchmarkScenario : public DBBenchmarkScenario {
  public:
    virtual void BenchInsertScenario(void* args = nullptr) override;
    
    virtual bool PrepareBenchmarkData() override;
};

void PGBenchmarkScenario::BenchInsertScenario(void* args) {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row;
        for (int i = 0; i < 16; i++) {
            row.emplace_back(GetNthAttr(line, i));
        }
        W.insert(row);
        count++;
    }

    W.complete();
    T.commit();
    in.close();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Insert " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

bool PGBenchmarkScenario::PrepareBenchmarkData() {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));

    cout << "Preparing benchmark data ..." << endl;
    for (string line; getline(in, line); ) {
        vector<string> row;
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