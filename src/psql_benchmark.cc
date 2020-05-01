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

using namespace std;
using namespace pqxx;

const char* kDATASOURCE = "DATASOURCE";
const char* kPGHOST = "PGHOST";
const char* kPGPORT = "PGPORT";
const char* kPGDATABASE = "PGDATABASE";
const char* kPGUSER = "PGUSER";
const char* kTableName = "LINEITEM";

int GetStrPos(const char *haystack, const char needle, const int n) {
    if (n == 0) {
        return -1;
    }

    const char *res = haystack;
    for (int i = 1; i <= n; ++i) {
        res = strchr(res, needle);
        if (!res) {
            return strlen(haystack);
        } else if (i != n) {
            res++;
        }
    }

    return res - haystack;
}

string GetNthAttr(const string& str, const int n) {
    int i = GetStrPos(str.c_str(), '|', n);
    int j = GetStrPos(str.c_str(), '|', n + 1);
    return string(str.substr(i + 1, j - i - 1));
}

int main(int argc, char** argv) {
    string host(getenv(kPGHOST));
    string port(getenv(kPGPORT));
    string user(getenv(kPGUSER));
    string db(getenv(kPGDATABASE));
    string table(kTableName);

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

    auto start = std::chrono::high_resolution_clock::now();
    ifstream in(kDATASOURCE);
    unsigned long long counter = 0;

    for (string line; getline(in, line); ) {
        vector<string> row;
        for (int i=0; i<16; i++) {
            string attr(GetNthAttr(line, i));
            row.push_back(attr);
        }

        W.insert(row);
        counter++;
    }

    W.complete();
    T.commit();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    std::cout << "Insert " << counter << " rows and takes "
              << ms.count() << " ms" << std::endl;

    return 0;
}
