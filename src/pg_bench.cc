#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/tablewriter>

#include "bench.h"

using namespace std;
using namespace pqxx;

int main(int argc, char** argv) {
    string host(getenv(kPGHOST));
    string port(getenv(kPGPORT));
    string user(getenv(kPGUSER));
    string db(getenv(kPGDATABASE));
    string table(kTABLE);
    string source(getenv(kDATASOURCE));

    // print db info
    cout << "********************" << endl;
    cout << " db host: " << host << endl;
    cout << " db port: " << port << endl;
    cout << " db user: " << user << endl;
    cout << " db name: " << db << endl;
    cout << " db table: " << table << endl;
    cout << " data source: " << source << endl;
    cout << "********************" << endl;

    // connect to db
    connection C("hostaddr=" + host + " port=" + port +
                 " dbname=" + db + " user=" + user);
    if (C.is_open()) {
        cout << "We are connected to " << C.dbname() << endl;
    } else {
        cout << "We are not connected!" << endl;
        return 0;
    }

    work T(C);
    tablewriter W(T, table);
    ifstream in(source);
    unsigned long long count = 0;

    cout << "Start to benchmark insertion rate ..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
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

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    double seconds = ms.count() / 1000;
    double tps = count / seconds;
    std::cout << "Insert " << count << " rows and take "
              << seconds << " s, tps = " << tps
              << std::endl;
    return 0;
}
