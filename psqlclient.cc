#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/tablewriter>
using namespace std;
using namespace pqxx;

string source = "/home/c8guo/wrapper/test/lineitemdata";

int strpos(const char *haystack, char needle, int n) {
    if (n == 0) return -1;
    const char *res = haystack;
    for (int i = 1; i <= n; ++i) {
        res = strchr(res, needle);
	if (!res) return strlen(haystack);
	else if (i != n) res++;
    }
    return res - haystack;
}

string GetNthAttr(const string& str, int n) {
    int i = strpos(str.c_str(), '|', n);
    int j = strpos(str.c_str(), '|', n + 1);
    return string(str.substr(i + 1, j - i - 1));
}

int main(int argc, char** argv) {
	connection C("dbname=testinsert user=postgres");
	string tableName("lineitem");

	if (C.is_open()) {
            cout << "We are connected to " << C.dbname() << endl;
	} else {
            cout << "We are not connected!" << endl;
	    return 0;
	}

	work T(C);
        tablewriter W(T, tableName);

        auto t1 = std::chrono::high_resolution_clock::now();
        ifstream in(source);
        int linecounter = 0;
	for (string line; getline(in, line); ) {
	    vector<string> tmpData;
	    for (int i=0;i<16;i++) {
		string nm = GetNthAttr(line, i);
	        tmpData.push_back(nm);
	    }
	    linecounter++;
	    W.insert(tmpData);
	}
	W.complete();
	T.commit();
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        std::cout<<"Insert takes "<<fp_ms.count() << " ms"<<std::endl;

	return 0;
}
