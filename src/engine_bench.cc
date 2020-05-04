#include <stdlib.h>
#include "engine_bench.h"

int main() {
    string dbpath(getenv(kDBPATH));
    
    // remove the existed db path
    int ret = system(string("rm -rf " + dbpath).c_str());
    PrintDBInfo();

    DB* db;
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;

    Status s = DB::Open(options, dbpath, &db);
    assert(s.ok());
    BenchPut(db);

    delete db;
    return 0;
}