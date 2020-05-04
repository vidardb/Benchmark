#ifndef BENCH_H_
#define BENCH_H_

#include <string>
#include <string.h>

using namespace std;

const char* kDATASOURCE = "DATASOURCE";
const char* kPGHOST = "PGHOST";
const char* kPGPORT = "PGPORT";
const char* kPGDATABASE = "PGDATABASE";
const char* kPGUSER = "PGUSER";
const char* kDBPATH = "DBPATH";
const char* kTABLE = "LINEITEM";

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

#endif /* BENCH_H_ */