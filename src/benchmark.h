#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <stdlib.h>
#include <pqxx/pqxx>
#include <pqxx/connection>

using namespace pqxx;

const char* kDataSource = "DATASOURCE";
const char* kPGHost = "PGHOST";
const char* kPGPort = "PGPORT";
const char* kPGDatabase = "PGDATABASE";
const char* kPGUser = "PGUSER";
const char* kDBPath = "DBPATH";
const char* kTableName = "LINEITEM";
const char* kPlatform = "PLATFORM";
const char* kScenario = "SCENARIO";

const std::string kPG = "pg";
const std::string kFDW = "fdw";
const std::string kEngine = "engine";

const std::string kInsert = "insert";
const std::string kScan = "scan";

class BenchmarkScenario {
  public:
    virtual ~BenchmarkScenario() {};

    virtual bool BeforeBenchmark(void* args = nullptr) { return true; };
    virtual bool AfterBenchmark(void* args = nullptr) { return true; };

    virtual void BenchInsertScenario(void* args = nullptr) {};
    virtual void BenchScanScenario(void* args = nullptr) {};

    virtual bool PrepareBenchmarkData() { return true; };
    virtual void DisplayBenchmarkInfo() {};
};

class DBBenchmarkScenario : public BenchmarkScenario {
  public:
    virtual ~DBBenchmarkScenario() { delete C; };

    virtual bool BeforeBenchmark(void* args = nullptr) override;

    virtual void BenchScanScenario(void* args = nullptr) override;

    virtual void DisplayBenchmarkInfo() override;

  protected:
    connection* C;
};

bool DBBenchmarkScenario::BeforeBenchmark(void* args) {
    std::string host(getenv(kPGHost));
    std::string port(getenv(kPGPort));
    std::string user(getenv(kPGUser));
    std::string db(getenv(kPGDatabase));

    C = new connection("hostaddr=" + host + " port=" + port +
                          " dbname=" + db + " user=" + user);
    if (C->is_open()) {
        std::cout << "We are connected to " << C->dbname() << std::endl;
        return true;
    } else {
        std::cout << "We are not connected!" << std::endl;
        return false;
    }
}

void DBBenchmarkScenario::BenchScanScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        std::cout << "Prepare data failed" << std::endl;
        return;
    }

    nontransaction T(*C);
    std::string value;
    unsigned long long count = 0;

    std::cout << "Start to benchmark iteration rate ..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    result res{ T.exec("SELECT * FROM " + std::string(kTableName)) };

    for (const auto& row: res) {
        for (const auto& col: row) {
            value.assign(col.c_str());
        }
        count++;
    }

    T.commit();  // nontransaction does nothing

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    std::cout << "Iterate " << count << " rows and take "
              << seconds << " s, tps = " << tps
              << std::endl;
}

void DBBenchmarkScenario::DisplayBenchmarkInfo() {
    std::string platform(getenv(kPlatform));
    std::string scenario(getenv(kScenario));
    std::string host(getenv(kPGHost));
    std::string port(getenv(kPGPort));
    std::string user(getenv(kPGUser));
    std::string db(getenv(kPGDatabase));
    std::string table(kTableName);
    std::string source(getenv(kDataSource));

    std::cout << "********************" << std::endl;
    std::cout << " platform: " << platform << std::endl;
    std::cout << " scenario: " << scenario << std::endl;
    std::cout << std::endl;
    std::cout << " db host: " << host << std::endl;
    std::cout << " db port: " << port << std::endl;
    std::cout << " db user: " << user << std::endl;
    std::cout << " db name: " << db << std::endl;
    std::cout << " db table: " << table << std::endl;
    std::cout << " data source: " << source << std::endl;
    std::cout << "********************" << std::endl;
}

#endif /* BENCHMARK_H_ */