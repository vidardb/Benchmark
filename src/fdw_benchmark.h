#ifndef FDW_BENCHMARK_H_
#define FDW_BENCHMARK_H_

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

class FDWBenchmarkScenario : public DBBenchmarkScenario {
  public:
    virtual void BenchInsertScenario(void* args = nullptr) override;

    virtual bool PrepareBenchmarkData() override;
};

void FDWBenchmarkScenario::BenchInsertScenario(void* args) {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row;
        string orderKey(GetNthAttr(line, 0));
        string lineNumber(GetNthAttr(line, 3));
        // composite key: (orderKey,lineNumber)
        row.emplace_back("("+orderKey+","+lineNumber+")");
        for (int i = 1; i < 16; i++) {
            if (i == 3) { // skip lineNumber
                continue;
            }
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

bool FDWBenchmarkScenario::PrepareBenchmarkData() {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));

    cout << "Preparing benchmark data ..." << endl;
    for (string line; getline(in, line); ) {
        vector<string> row;
        string orderKey(GetNthAttr(line, 0));
        string lineNumber(GetNthAttr(line, 3));
        // composite key: (orderKey,lineNumber)
        row.emplace_back("("+orderKey+","+lineNumber+")");
        for (int i = 1; i < 16; i++) {
            if (i == 3) { // skip lineNumber
                continue;
            }
            row.emplace_back(GetNthAttr(line, i));
        }
        W.insert(row);
    }

    W.complete();
    T.commit();
    in.close();
    return true;
}

BenchmarkScenario* NewFDWBenchmarkScenario() {
    return new FDWBenchmarkScenario();
}

#endif  /* FDW_BENCHMARK_H_ */