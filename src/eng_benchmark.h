#ifndef ENG_BENCHMARK_H_
#define ENG_BENCHMARK_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
using namespace std;

#include "vidardb/db.h"
#include "vidardb/options.h"
#include "vidardb/table.h"
#include "benchmark.h"
#include "util.h"
using namespace vidardb;

const string delim = "|";

void PutFixed32(string* dst, uint32_t value);
void EncodeAttr(const string& s, string& k, string& v);
void DecodeAttr(const string& attr);

class EngBenchmarkScenario : public BenchmarkScenario {
  public:
    virtual ~EngBenchmarkScenario() { delete db; };

    virtual bool BeforeBenchmark(void* args = nullptr) override;

    virtual void BenchInsertScenario(void* args = nullptr) override;
    virtual void BenchLoadScenario(void* args = nullptr) override;
    virtual void BenchScanScenario(void* args = nullptr) override;
    virtual void BenchGetScenario(void* args = nullptr) override;
    
    virtual bool PrepareBenchmarkData() override;
    virtual void DisplayBenchmarkInfo() override;

  private:
    DB* db;
};

bool EngBenchmarkScenario::BeforeBenchmark(void* args) {
    string dbpath(getenv(kDBPath));

    int ret = system(string("rm -rf " + dbpath).c_str());
    if (ret != 0) {
        cout << "remove engine dbpath failed" << endl;
    }

    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    Status s = DB::Open(options, dbpath, &db);
    return s.ok(); 
}

void EngBenchmarkScenario::BenchInsertScenario(void* args) {
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        string key, value;
        EncodeAttr(line.substr(0, line.size()-1), key, value);

        Status s = db->Put(WriteOptions(), key, value);
        if (!s.ok()) {
            cout << s.ToString() << endl;
        }
        count++;
    }

    auto end = chrono::high_resolution_clock::now();
    db->Flush(FlushOptions());
    in.close();

    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Insert " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl; 
}

void EngBenchmarkScenario::BenchLoadScenario(void* args) {
    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        string key, value;
        EncodeAttr(line.substr(0, line.size()-1), key, value);

        Status s = db->Put(WriteOptions(), key, value);
        if (!s.ok()) {
            cout << s.ToString() << endl;
        }
        count++;
    }

    auto end = chrono::high_resolution_clock::now();
    db->Flush(FlushOptions());
    in.close();

    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Insert " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl; 
}

bool EngBenchmarkScenario::PrepareBenchmarkData() {
    ifstream in(string(getenv(kDataSource)));

    cout << "Preparing benchmark data ..." << endl;
    for (string line; getline(in, line); ) {
        string key, value;
        EncodeAttr(line.substr(0, line.size()-1), key, value);

        Status s = db->Put(WriteOptions(), key, value);
        if (!s.ok()) {
            cout << s.ToString() << endl;
        }
    }

    in.close();
    return true;
}

void EngBenchmarkScenario::BenchScanScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }

    string key, value;
    ReadOptions ro;
    unsigned long long count = 0;
    Iterator *it = db->NewIterator(ro);

    cout << "Start to benchmark iteration rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        key.assign(it->key().data(), it->key().size());
        value.assign(it->value().data(), it->value().size());
	    DecodeAttr(value);
        count++;
    }

    auto end = chrono::high_resolution_clock::now();
    delete it;

    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Iterate " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void EngBenchmarkScenario::BenchGetScenario(void* args) {
    vector<pair<string, string>> v;
    PrepareGetData(v);
    ifstream in(getenv(kDataSource));

    auto start = chrono::high_resolution_clock::now();
    for (auto& t : v) {
        string orderKey, lineNumber;
        PutFixed32(&orderKey, stoul(t.first));
        PutFixed32(&lineNumber, stoul(t.second));
        string key(orderKey + delim + lineNumber);
        ReadOptions ro;
        string val;
        db->Get(ro, key, &val);
    }

    in.close();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = v.size() / seconds;
    cout << "Get " << v.size() << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void EngBenchmarkScenario::DisplayBenchmarkInfo() {
    string platform(getenv(kPlatform));
    string scenario(getenv(kScenario));
    string source(getenv(kDataSource));
    string dbpath(getenv(kDBPath));

    cout << "********************" << endl;
    cout << " platform: " << platform << endl;
    cout << " scenario: " << scenario << endl;
    cout << endl;
    cout << " db path: " << dbpath << endl;
    cout << " data source: " << source << endl;
    cout << "********************" << endl;
}

void PutFixed32(string* dst, uint32_t value) {
    char buf[sizeof(value)];
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;
    dst->append(buf, sizeof(buf));
}

void EncodeAttr(const string& s, string& k, string& v) {
    string orderKey, lineNumber;
    PutFixed32(&orderKey, stoul(GetNthAttr(s, 0)));
    PutFixed32(&lineNumber, stoul(GetNthAttr(s, 3)));

    string partKey(GetNthAttr(s, 1));
    string suppKey(GetNthAttr(s, 2));
    string quantity(GetNthAttr(s, 4));
    string extendedPrice(GetNthAttr(s, 5));
    string discount(GetNthAttr(s, 6));
    string tax(GetNthAttr(s, 7));
    string returnFlag(GetNthAttr(s, 8));
    string lineStatus(GetNthAttr(s, 9));
    string shipDate(GetNthAttr(s, 10));
    string commitDate(GetNthAttr(s, 11));
    string receiptDate(GetNthAttr(s, 12));
    string shipInstruct(GetNthAttr(s, 13));
    string shipMode(GetNthAttr(s, 14));
    string comment(GetNthAttr(s, 15));

    k.assign(orderKey + delim + lineNumber);
    v.assign(partKey + delim + suppKey + delim + quantity + delim
        + extendedPrice + delim + discount + delim + tax + delim
        + returnFlag + delim + lineStatus + delim + shipDate
        + delim + commitDate + delim + receiptDate + delim
        + shipInstruct + delim + shipMode + delim + comment);
}

void DecodeAttr(const string& attr) {
    size_t last = 0, next = 0;
    while ((next = attr.find(delim, last)) != string::npos) {
        attr.substr(last, next - last);
        last = next + 1;
    }
    attr.substr(last);
}

BenchmarkScenario* NewEngBenchmarkScenario() {
    return new EngBenchmarkScenario();
}

#endif /* ENG_BENCHMARK_H_ */