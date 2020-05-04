#ifndef ENGINE_BENCH_H_
#define ENGINE_BENCH_H_

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unistd.h>

#include "vidardb/db.h"
#include "vidardb/options.h"
#include "vidardb/table.h"
#include "bench.h"

using namespace std;
using namespace std::chrono;
using namespace vidardb;

void PrintDBInfo() {
    string source(getenv(kDATASOURCE));
    string dbpath(getenv(kDBPATH));

    cout << "********************" << endl;
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

void EncodeLine(const string& line, string& k, string& v) {
    string orderKey, lineNumber;
    PutFixed32(&orderKey, stoul(GetNthAttr(line, 0)));
    PutFixed32(&lineNumber, stoul(GetNthAttr(line, 3)));

    string partKey(GetNthAttr(line, 1));
    string suppKey(GetNthAttr(line, 2));
    string quantity(GetNthAttr(line, 4));
    string extendedPrice(GetNthAttr(line, 5));
    string discount(GetNthAttr(line, 6));
    string tax(GetNthAttr(line, 7));
    string returnFlag(GetNthAttr(line, 8));
    string lineStatus(GetNthAttr(line, 9));
    string shipDate(GetNthAttr(line, 10));
    string commitDate(GetNthAttr(line, 11));
    string receiptDate(GetNthAttr(line, 12));
    string shipInstruct(GetNthAttr(line, 13));
    string shipMode(GetNthAttr(line, 14));
    string comment(GetNthAttr(line, 15));

    k.assign(orderKey + "|" + lineNumber);
    v.assign(partKey + "|" + suppKey + "|" + quantity + "|"
        + extendedPrice + "|" + discount + "|" + tax + "|"
        + returnFlag + "|" + lineStatus + "|"
        + shipDate + "|" + commitDate + "|" + receiptDate + "|"
        + shipInstruct + "|" + shipMode + "|" + comment);
}

void BenchPut(DB* db) {
    unsigned long long count = 0;
    string source(getenv(kDATASOURCE));
    ifstream in(source);

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (string line; getline(in, line); ) {
        string k, v;
        EncodeLine(line.substr(0, line.size()-1), k, v);

        Status s = db->Put(WriteOptions(), k, v);
        if (!s.ok()) {
            cout << s.ToString() << endl;
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

#endif /* ENGINE_BENCH_H_ */