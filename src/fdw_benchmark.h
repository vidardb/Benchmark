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

    virtual void BenchLoadScenario(void* args = nullptr) override;

    virtual void BenchGetScenario(GetType type) override;

    virtual bool PrepareBenchmarkData() override;

 protected:
    vector<string> EncodeTuple(const string& line);
};

void FDWBenchmarkScenario::BenchInsertScenario(void* args) {
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    work T(*C);
    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row(EncodeTuple(line));

        string stmt = "INSERT INTO LINEITEM VALUES(";
        stmt += row[0] + ", " + row[1] + ", " + row[2] +  ", " + row[3] + ", " + 
                row[4] + ", " + row[5] + ", " + row[6] + ", '" + row[7] + "', '" 
                + row[8] + "', '" + row[9] + "', '" + row[10]  + "', '" + 
                row[11] + "', '" + row[12]  + "', '" + row[13] + "', '" + 
                row[14] + "');";
        pqxx::result res = T.exec(stmt);
        count++;
    }
    T.commit();
    in.close();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Insert " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void FDWBenchmarkScenario::BenchLoadScenario(void* args) {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
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

void FDWBenchmarkScenario::BenchGetScenario(GetType type) {
    vector<pair<string, string>> v;
    PrepareGetData(v, type);
    ifstream in(getenv(kDataSource));

    auto start = chrono::high_resolution_clock::now();
    work T(*C);
    for (auto& t : v) {
        string stmt = "SELECT * FROM LINEITEM WHERE ";
        stmt += "L_ORDERKEY = (" + t.first;
        stmt += ", " + t.second + ")";
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

bool FDWBenchmarkScenario::PrepareBenchmarkData() {
    work T(*C);
    tablewriter W(T, string(kTableName));
    ifstream in(string(getenv(kDataSource)));

    cout << "Preparing benchmark data ..." << endl;
    for (string line; getline(in, line); ) {
        vector<string> row(EncodeTuple(line));
        W.insert(row);
    }

    W.complete();
    T.commit();
    in.close();
    return true;
}

vector<string> FDWBenchmarkScenario::EncodeTuple(const string& line) {
    vector<string> row;
    row.reserve(15);
    size_t last = 0, next = 0;
    int i = 0;
    while ((next = line.find(delim, last)) != string::npos) {
        string attr = line.substr(last, next - last);
        // composite key: (orderKey,lineNumber)
        if (i == 0) {
            row.push_back("(" + attr + ",");
        } else if (i == 3) {
            row[0].append(attr + ")");
        } else {
            row.emplace_back(attr);
        }

        last = next + 1;
        ++i;
    }
    return row;
}

BenchmarkScenario* NewFDWBenchmarkScenario() {
    return new FDWBenchmarkScenario();
}

#endif  /* FDW_BENCHMARK_H_ */