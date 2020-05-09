#ifndef ENG_BENCHMARK_H_
#define ENG_BENCHMARK_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <stdlib.h>
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/tablewriter>

#include "vidardb/db.h"
#include "vidardb/options.h"
#include "vidardb/table.h"
#include "benchmark.h"
#include "util.h"

using namespace vidardb;

const std::string delim = "|";

void PutFixed32(std::string* dst, uint32_t value);
void EncodeAttr(const std::string& s, std::string& k, std::string& v);
void DecodeAttr(const std::string& attr);

class EngBenchmarkScenario : public BenchmarkScenario {
  public:
    virtual ~EngBenchmarkScenario() { delete db; };

    using BenchmarkScenario::BeforeBenchmark;
    virtual bool BeforeBenchmark(void* args = nullptr) override;

    using BenchmarkScenario::BenchInsertScenario;
    virtual void BenchInsertScenario(void* args = nullptr) override;

    using BenchmarkScenario::BenchScanScenario;
    virtual void BenchScanScenario(void* args = nullptr) override;

    using BenchmarkScenario::PrepareBenchmarkData;
    virtual bool PrepareBenchmarkData() override;

    using BenchmarkScenario::DisplayBenchmarkInfo;
    virtual void DisplayBenchmarkInfo() override;

  private:
    DB* db;
};

bool EngBenchmarkScenario::BeforeBenchmark(void* args) {
    std::string dbpath(getenv(kDBPath));

    int ret = system(std::string("rm -rf " + dbpath).c_str());
    if (ret != 0) {
        std::cout << "remove engine dbpath failed" << std::endl;
    }

    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;

    Status s = DB::Open(options, dbpath, &db);
    return s.ok(); 
}

void EngBenchmarkScenario::BenchInsertScenario(void* args) {
    std::ifstream in(std::string(getenv(kDataSource)));
    unsigned long long count = 0;

    std::cout << "Start to benchmark insertion rate ..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (std::string line; getline(in, line); ) {
        std::string key, value;
        EncodeAttr(line.substr(0, line.size()-1), key, value);

        Status s = db->Put(WriteOptions(), key, value);
        if (!s.ok()) {
            std::cout << s.ToString() << std::endl;
        }
        count++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    db->Flush(FlushOptions());
    in.close();

    std::chrono::duration<double, std::milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    std::cout << "Insert " << count << " rows and take "
              << seconds << " s, tps = " << tps
              << std::endl; 
}

bool EngBenchmarkScenario::PrepareBenchmarkData() {
    std::ifstream in(std::string(getenv(kDataSource)));

    std::cout << "Preparing benchmark data ..." << std::endl;
    for (std::string line; getline(in, line); ) {
        std::string key, value;
        EncodeAttr(line.substr(0, line.size()-1), key, value);

        Status s = db->Put(WriteOptions(), key, value);
        if (!s.ok()) {
            std::cout << s.ToString() << std::endl;
        }
    }

    in.close();
    return true;
}

void EngBenchmarkScenario::BenchScanScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        std::cout << "Prepare benchmark data failed" << std::endl;
        return;
    }

    std::string key, value;
    ReadOptions ro;
    unsigned long long count = 0;
    Iterator *it = db->NewIterator(ro);

    std::cout << "Start to benchmark iteration rate ..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        key.assign(it->key().data(), it->key().size());
        value.assign(it->value().data(), it->value().size());
	    DecodeAttr(value);
        count++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    delete it;

    std::chrono::duration<double, std::milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    std::cout << "Iterate " << count << " rows and take "
              << seconds << " s, tps = " << tps
              << std::endl;
}

void EngBenchmarkScenario::DisplayBenchmarkInfo() {
    std::string platform(getenv(kPlatform));
    std::string scenario(getenv(kScenario));
    std::string source(getenv(kDataSource));
    std::string dbpath(getenv(kDBPath));

    std::cout << "********************" << std::endl;
    std::cout << " platform: " << platform << std::endl;
    std::cout << " scenario: " << scenario << std::endl;
    std::cout << std::endl;
    std::cout << " db path: " << dbpath << std::endl;
    std::cout << " data source: " << source << std::endl;
    std::cout << "********************" << std::endl;
}

void PutFixed32(std::string* dst, uint32_t value) {
    char buf[sizeof(value)];
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;
    dst->append(buf, sizeof(buf));
}

void EncodeAttr(const std::string& s, std::string& k, std::string& v) {
    std::string orderKey, lineNumber;
    PutFixed32(&orderKey, std::stoul(GetNthAttr(s, 0)));
    PutFixed32(&lineNumber, std::stoul(GetNthAttr(s, 3)));

    std::string partKey(GetNthAttr(s, 1));
    std::string suppKey(GetNthAttr(s, 2));
    std::string quantity(GetNthAttr(s, 4));
    std::string extendedPrice(GetNthAttr(s, 5));
    std::string discount(GetNthAttr(s, 6));
    std::string tax(GetNthAttr(s, 7));
    std::string returnFlag(GetNthAttr(s, 8));
    std::string lineStatus(GetNthAttr(s, 9));
    std::string shipDate(GetNthAttr(s, 10));
    std::string commitDate(GetNthAttr(s, 11));
    std::string receiptDate(GetNthAttr(s, 12));
    std::string shipInstruct(GetNthAttr(s, 13));
    std::string shipMode(GetNthAttr(s, 14));
    std::string comment(GetNthAttr(s, 15));

    k.assign(orderKey + delim + lineNumber);
    v.assign(partKey + delim + suppKey + delim + quantity + delim
        + extendedPrice + delim + discount + delim + tax + delim
        + returnFlag + delim + lineStatus + delim + shipDate
        + delim + commitDate + delim + receiptDate + delim
        + shipInstruct + delim + shipMode + delim + comment);
}

void DecodeAttr(const std::string& attr) {
    size_t last = 0, next = 0;
    while ((next = attr.find(delim, last)) != std::string::npos) {
        attr.substr(last, next - last);
        last = next + 1;
    }
    attr.substr(last);
}

BenchmarkScenario* NewEngBenchmarkScenario() {
    return new EngBenchmarkScenario();
}

#endif /* ENG_BENCHMARK_H_ */