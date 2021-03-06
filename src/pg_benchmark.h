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

    virtual void BenchGetScenario(GetType type) override;
    
    virtual bool PrepareBenchmarkData() override;

 protected:
    vector<string> EncodeTuple(const string& line);
};

void PGBenchmarkScenario::BenchInsertScenario(void* args) {
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    work T(*C);
    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        vector<string> row(EncodeTuple(line));

        string stmt = "INSERT INTO LINEITEM VALUES(";
        stmt += row[0] + ", " + row[1] + ", " + row[2] +  ", " + row[3] + ", " + 
                row[4] + ", " + row[5] + ", " + row[6] + ", " + row[7] + ", '" + 
                row[8] + "', '" + row[9] + "', '" + row[10] + "', '" + row[11] + 
                "', '" + row[12]  + "', '" + row[13] + "', '" + row[14] + "', '" 
                + row[15] + "');";
        // cout<<stmt<<endl;
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

void PGBenchmarkScenario::BenchLoadScenario(void* args) {
    work T(*C);
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

void PGBenchmarkScenario::BenchGetScenario(GetType type) {
    C->prepare("get", "SELECT * FROM LINEITEM WHERE L_ORDERKEY = $1 AND L_LINENUMBER = $2;");

    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }

    vector<pair<string, string>> v;
    PrepareGetData(v, type);
    ifstream in(getenv(kDataSource));

    cout << "Start to benchmark get rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    work T(*C);
    for (auto& t : v) {
        // string stmt = "SELECT * FROM LINEITEM WHERE ";
        // stmt += "L_ORDERKEY = " + t.first;
        // stmt += " AND L_LINENUMBER = " + t.second;
        // cout<<stmt<<endl;
        // pqxx::result res = T.exec(stmt);
        pqxx::result res = T.prepared("get")(t.first)(t.second).exec();
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
        vector<string> row(EncodeTuple(line));
        W.insert(row);
    }

    W.complete();
    T.commit();
    in.close();
    return true;
}

vector<string> PGBenchmarkScenario::EncodeTuple(const string& line) {
    vector<string> row;
    row.reserve(16);
    size_t last = 0, next = 0;
    while ((next = line.find(delim, last)) != string::npos) {
        row.emplace_back(line.substr(last, next - last));
        last = next + 1;
    }
    return row;
}

BenchmarkScenario* NewPGBenchmarkScenario() {
    return new PGBenchmarkScenario();
}

#endif  /* PG_BENCHMARK_H_ */