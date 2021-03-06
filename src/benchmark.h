#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <set>
using namespace std;

#include <pqxx/pqxx>
#include <pqxx/connection>
using namespace pqxx;

#include "util.h"


const char* kDataSource = "DATASOURCE";
const char* kPGHost = "PGHOST";
const char* kPGPort = "PGPORT";
const char* kPGDatabase = "PGDATABASE";
const char* kPGUser = "PGUSER";
const char* kDBPath = "DBPATH";
const char* kTableName = "LINEITEM";
const char* kPlatform = "PLATFORM";
const char* kScenario = "SCENARIO";

const string kPG = "pg";
const string kFDW = "fdw";
const string kTBAM = "tbam";
const string kEngine = "engine";

const string kInsert = "insert";
const string kLoad = "load";
const string kScan = "scan";
const string kGetRandom = "getrand";
const string kGetLast = "getlast";

const int kGetCount = 100000;
const string delim = "|";

enum GetType {
    GetRand, 
    GetLast,
};


class BenchmarkScenario {
  public:
    
    virtual ~BenchmarkScenario() {}

    virtual bool BeforeBenchmark(void* args = nullptr) { return true; }
    virtual bool AfterBenchmark(void* args = nullptr) { return true; }

    virtual void BenchInsertScenario(void* args = nullptr) {}
    virtual void BenchLoadScenario(void* args = nullptr) {}
    virtual void BenchScanScenario(void* args = nullptr) {}
    virtual void BenchGetScenario(GetType type) {}

    virtual bool PrepareBenchmarkData() { return true; }
    virtual void DisplayBenchmarkInfo() {}

    virtual void PrepareGetData(vector<pair<string, string>>& v, GetType type) {
        unsigned long long count = GetLineCount(getenv(kDataSource));
        set<int> s;
        if (type == GetType::GetRand) {
            for (int i=0; i<kGetCount; ++i) {
                s.insert(rand()%count);
            }
        } else if (type == GetType::GetLast) {
            for (int i=count-kGetCount; i<count; ++i) {
                s.insert(i);
            }
        }

        ifstream in(getenv(kDataSource));
        string line;
        for (int i = 0; std::getline(in, line); ++i) {
            if (s.find(i) == s.end()) continue;
            v.push_back({GetNthAttr(line, 0), GetNthAttr(line, 3)});        
        }
        in.close();
    }
};

class DBBenchmarkScenario : public BenchmarkScenario {
  public:
    virtual ~DBBenchmarkScenario() { delete C; }

    virtual bool BeforeBenchmark(void* args = nullptr) override;

    virtual void BenchScanScenario(void* args = nullptr) override;

    virtual void DisplayBenchmarkInfo() override;

  protected:
    vector<string> EncodeTuple(const string& line) { return vector<string>(); }

    connection* C;
};

bool DBBenchmarkScenario::BeforeBenchmark(void* args) {
    string host(getenv(kPGHost));
    string port(getenv(kPGPort));
    string user(getenv(kPGUser));
    string db(getenv(kPGDatabase));

    C = new connection("hostaddr=" + host + " port=" + port +
                          " dbname=" + db + " user=" + user);
    if (C->is_open()) {
        cout << "We are connected to " << C->dbname() << endl;
        return true;
    } else {
        cout << "We are not connected!" << endl;
        return false;
    }
}

void DBBenchmarkScenario::BenchScanScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }

    work T(*C);
    string value;
    unsigned long long count = 0;

    cout << "Start to benchmark iteration rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    result res = T.exec("SELECT * FROM " + string(kTableName));
    T.commit();

    for (const auto& row: res) {
        for (const auto& col: row) {
            value.assign(col.c_str());
        }
        count++;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Iterate " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void DBBenchmarkScenario::DisplayBenchmarkInfo() {
    string platform(getenv(kPlatform));
    string scenario(getenv(kScenario));
    string host(getenv(kPGHost));
    string port(getenv(kPGPort));
    string user(getenv(kPGUser));
    string db(getenv(kPGDatabase));
    string table(kTableName);
    string source(getenv(kDataSource));

    cout << "********************" << endl;
    cout << " platform: " << platform << endl;
    cout << " scenario: " << scenario << endl;
    cout << endl;
    cout << " db host: " << host << endl;
    cout << " db port: " << port << endl;
    cout << " db user: " << user << endl;
    cout << " db name: " << db << endl;
    cout << " db table: " << table << endl;
    cout << " data source: " << source << endl;
    cout << "********************" << endl;
}

#endif /* BENCHMARK_H_ */