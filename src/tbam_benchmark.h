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
  public:
    virtual void BenchLoadScenario(void* args = nullptr) override;
};

void TBAMBenchmarkScenario::BenchLoadScenario(void* args) {
    work T(*C);
    
    string stmt = "set kv_am.engine_options='lineitem:disable_auto_compactions:true'";
    pqxx::result res = T.exec(stmt);

    stmt = "set kv_am.engine_options='lineitem:write_buffer_size:50331648'";
    res = T.exec(stmt);

    /* 
     * TODO: investigate the reason of perf difference between setting in options and here
     * becasue we find 225k vs. 170k here.
     */
    // stmt = "set kv_am.engine_options='lineitem:max_write_buffer_number:2'";
    // res = T.exec(stmt);

    stmt = "set kv_am.engine_options='lineitem:level0_file_num_compaction_trigger:1073741824'";
    res = T.exec(stmt);

    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark loading rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row(EncodeTuple(line));
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
    cout << "Load " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

BenchmarkScenario* NewTBAMBenchmarkScenario() {
    return new TBAMBenchmarkScenario();
}

#endif  /* TBAM_BENCHMARK_H_ */