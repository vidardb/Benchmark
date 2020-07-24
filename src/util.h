#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <cstring>
using namespace std;

int IndexOf(const char *str, const char c, const int n) {
    if (n == 0) {
        return -1;
    }

    const char *res = str;
    for (int i = 1; i <= n; ++i) {
        res = strchr(res, c);
        if (!res) {
            return strlen(str);
        } else if (i != n) {
            res++;
        }
    }

    return res - str;
}

string GetNthAttr(const string& str, const int n) {
    int i = IndexOf(str.c_str(), '|', n);
    int j = IndexOf(str.c_str(), '|', n + 1);
    return string(str.substr(i + 1, j - i - 1));
}

bool StringEquals(const string& a, const string& b) {
    return (a == b);
}

#endif /* UTIL_H_ */