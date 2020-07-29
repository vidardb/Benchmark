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

    virtual void BenchLoadScenario(void* args = nullptr) override;

    virtual void BenchGetScenario(void* args = nullptr) override;
    
    virtual bool PrepareBenchmarkData() override;
};

void PGBenchmarkScenario::BenchInsertScenario(void* args) {
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row;
        for (int i = 0; i < 16; i++) {
            row.emplace_back(GetNthAttr(line, i));
        }
        work T(*C);
        string stmt = "INSERT INTO LINEITEM VALUES(";
        stmt += row[0] + ", " + row[1] + ", " + row[2] +  ", " + row[3] + ", " + 
                row[4] + ", " + row[5] + ", " + row[6] + ", " + row[7] + ", '" + 
                row[8] + "', '" + row[9] + "', '" + row[10] + "', '" + row[11] + 
                "', '" + row[12]  + "', '" + row[13] + "', '" + row[14] + "', '" 
                + row[15] + "');";
        // cout<<stmt<<endl;
        pqxx::result res = T.exec(stmt);
        T.commit();
        cout<<count++<<endl;
    }
 
    in.close();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Insert " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void PGBenchmarkScenario::BenchLoadScenario(void* args) {
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

void PGBenchmarkScenario::BenchGetScenario(void* args) {
    vector<pair<string, string>> v;
    PrepareGetData(v);
    ifstream in(getenv(kDataSource));

    auto start = chrono::high_resolution_clock::now();
    work T(*C);
    for (auto& t : v) {
        string stmt = "SELECT * FROM LINEITEM WHERE ";
        stmt += "L_ORDERKEY = " + t.first;
        stmt += " AND L_LINENUMBER = " + t.second;
        // cout<<stmt<<endl;
        pqxx::result res = T.exec(stmt);
    }
    T.commit();
    in.close();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = v.size() / seconds;
    cout << "Get " << v.size() << " rows and take "
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