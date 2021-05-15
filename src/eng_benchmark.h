#ifndef ENG_BENCHMARK_H_
#define ENG_BENCHMARK_H_

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include "vidardb/db.h"
#include "vidardb/options.h"
#include "vidardb/table.h"
#include "vidardb/cache.h"
#include "vidardb/file_iter.h"
#include "benchmark.h"
#include "util.h"
using namespace vidardb;

void PutFixed32(string* dst, uint32_t value);

class EngBenchmarkScenario : public BenchmarkScenario {
  public:
    virtual ~EngBenchmarkScenario() { delete db; };

    virtual bool BeforeBenchmark(void* args = nullptr) override;

    virtual void BenchInsertScenario(void* args = nullptr) override;
    virtual void BenchLoadScenario(void* args = nullptr) override;
    virtual void BenchScanScenario(void* args = nullptr) override;
    virtual void BenchGetScenario(GetType type) override;
    virtual void BenchRangeQueryScenario(void* args = nullptr) override;

    virtual bool PrepareBenchmarkData() override;
    virtual void DisplayBenchmarkInfo() override;

  private:
    void EncodeTuple(const string& s, string& k, string& v);
    string DecodeTuple(const string& s);

    void Get(int n, GetType type);

    DB* db;
};

bool EngBenchmarkScenario::BeforeBenchmark(void* args) {
    string dbpath(getenv(kDBPath));

    int ret = system(string("rm -rf " + dbpath).c_str());
    if (ret != 0) {
        cout << "remove engine dbpath failed" << endl;
    }

    Options options;
    // options.IncreaseParallelism();
    // options.OptimizeLevelStyleCompaction();
    // options.PrepareForBulkLoad();
    BlockBasedTableOptions block_based_options;
    block_based_options.block_cache =
        NewLRUCache(static_cast<size_t>(96 * 1024 * 1024));
    options.table_factory.reset(NewBlockBasedTableFactory(block_based_options));
    options.OptimizeAdaptiveLevelStyleCompaction(128*1024*1024);

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
        EncodeTuple(line, key, value);

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
    bool disable_auto_compactions = db->GetOptions().disable_auto_compactions;
    if (!disable_auto_compactions) {
        db->SetOptions({{"disable_auto_compactions", "true"}});
    }

    ifstream in(string(getenv(kDataSource)));
    unsigned long long count = 0;
    int64_t entries_per_batch_ = 50;

    cout << "Start to benchmark loading rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();

    WriteBatch batch;
    WriteOptions opts;
    for (string line; getline(in, line);) {
        string key, value;
        EncodeTuple(line, key, value);

        if (count != 0 && count % entries_per_batch_ == 0) {
            Status s = db->Write(opts, &batch);
            if (!s.ok()) {
                cout << s.ToString() << endl;
                exit(1);
            }
            batch.Clear();
        }
        batch.Put(key, value);
        count++;
    }

    if (batch.Count() > 0) {
        Status s = db->Write(WriteOptions(), &batch);
        if (!s.ok()) {
            cout << s.ToString() << endl;
            exit(1);
        }
        batch.Clear();
    }

    auto end = chrono::high_resolution_clock::now();
    // db->Flush(FlushOptions());
    in.close();

    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Load " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;

    // if (!disable_auto_compactions) {
    //     db->EnableAutoCompaction({db->DefaultColumnFamily()});
    // }
}

bool EngBenchmarkScenario::PrepareBenchmarkData() {
    cout << "Start to prepare benchmark data ..." << endl;
    BenchLoadScenario();
    return true;
}

void EngBenchmarkScenario::BenchScanScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }
    db->CompactRange(CompactRangeOptions(), nullptr, nullptr);

    string key, value;
    ReadOptions ro;
    unsigned long long count = 0;
    Iterator *it = db->NewIterator(ro);

    cout << "Start to benchmark iteration rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        key.assign(it->key().data(), it->key().size());
        value.assign(it->value().data(), it->value().size());
	    DecodeTuple(value);
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

void EngBenchmarkScenario::BenchRangeQueryScenario(void* args) {
    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }
    db->CompactRange(CompactRangeOptions(), nullptr, nullptr);

    string value;
    ReadOptions ro;
    ro.columns.push_back(6); // tax
    unsigned long long count = 0, index = 1;
    FileIter *it = static_cast<FileIter*>(db->NewFileIterator(ro));
    vector<bool> block_bits; // full scan

    cout << "Start to benchmark range query rate ..." << endl;
    auto start = chrono::high_resolution_clock::now();

    for (it->SeekToFirst(); it->Valid(); it->Next(), index++) {
        vector<vector<MinMax>> min_max;
        auto mix_max_start = chrono::high_resolution_clock::now();
        Status s = it->GetMinMax(min_max);
        assert(s.ok());
        auto min_max_end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> min_max_ms =
            min_max_end - mix_max_start;
        cout << "MinMax" << index << ": " << min_max_ms.count() << " ms" << endl;

        vector<RangeQueryKeyVal> res;
        auto range_query_start = chrono::high_resolution_clock::now();
        s = it->RangeQuery(block_bits, res);
        assert(s.ok());
        auto range_query_end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> range_query_ms =
            range_query_end - range_query_start;
        cout << "RangeQuery" << index << ": " << range_query_ms.count() << " ms"
             << endl;

        for (RangeQueryKeyVal& kv : res) {
            value.assign(it->value().data(), it->value().size());
            DecodeTuple(value);
            count++;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    delete it;

    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    cout << "Range query " << count << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void EngBenchmarkScenario::Get(int n, GetType type) {
    vector<pair<string, string>> v;
    PrepareGetData(v, type, n);

    cout << "Start to benchmark get rate ..." << endl;
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
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = v.size() / seconds;
    cout << "Get " << v.size() << " rows and take "
         << seconds << " s, tps = " << tps << endl;
}

void EngBenchmarkScenario::BenchGetScenario(GetType type) {
    if (!PrepareBenchmarkData()) {
        cout << "Prepare data failed" << endl;
        return;
    }

    if (kWarmCount > 0) {
        cout << "Start to warmup ..." << endl;
        Get(kWarmCount, GetRand);
    }

    uint64_t n = kGetCount;
    if (type == GetLast) {
        uint64_t num;
        db->GetIntProperty(DB::Properties::kNumEntriesActiveMemTable, &num);
        cout << "kNumEntriesActiveMemTable: " << num << endl;
        n = min(num, n);
    }
    
    Get(n, type);
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

void EngBenchmarkScenario:: EncodeTuple(const string& s, string& k, string& v) {
    vector<string> t;
    t.reserve(16);
    size_t last = 0, next = 0;
    while ((next = s.find(delim, last)) != string::npos) {
        t.push_back(s.substr(last, next - last));
        last = next + 1;
    }

    string orderKey, lineNumber;
    PutFixed32(&orderKey, stoi(t[0]));
    PutFixed32(&lineNumber, stoi(t[3]));

    string partKey(move(t[1]));
    string suppKey(move(t[2]));
    string quantity(move(t[4]));
    string extendedPrice(move(t[5]));
    string discount(move(t[6]));
    string tax(move(t[7]));
    string returnFlag(move(t[8]));
    string lineStatus(move(t[9]));
    string shipDate(move(t[10]));
    string commitDate(move(t[11]));
    string receiptDate(move(t[12]));
    string shipInstruct(move(t[13]));
    string shipMode(move(t[14]));
    string comment(move(t[15]));

    k.assign(orderKey + delim + lineNumber);
    v.assign(partKey + delim + suppKey + delim + quantity + delim
        + extendedPrice + delim + discount + delim + tax + delim
        + returnFlag + delim + lineStatus + delim + shipDate
        + delim + commitDate + delim + receiptDate + delim
        + shipInstruct + delim + shipMode + delim + comment);
}

string EngBenchmarkScenario::DecodeTuple(const string& s) {
    string res;
    size_t last = 0, next = 0;
    while ((next = s.find(delim, last)) != string::npos) {
        res.append(s.substr(last, next - last));
        last = next + 1;
    }
    res.append(s.substr(last));
    return res;
}

BenchmarkScenario* NewEngBenchmarkScenario() {
    return new EngBenchmarkScenario();
}

#endif /* ENG_BENCHMARK_H_ */