#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <string.h>
#include <stdlib.h>

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

std::string GetNthAttr(const std::string& str, const int n) {
    int i = IndexOf(str.c_str(), '|', n);
    int j = IndexOf(str.c_str(), '|', n + 1);
    return std::string(str.substr(i + 1, j - i - 1));
}

bool StringEquals(const std::string& a, const std::string& b) {
    return a.compare(b) == 0;
}

#endif /* UTIL_H_ */