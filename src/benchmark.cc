#include <iostream>
#include <stdlib.h>

#include "benchmark.h"
#include "pg_benchmark.h"
#include "fdw_benchmark.h"
#include "eng_benchmark.h"
#include "util.h"

BenchmarkScenario* NewBenchmarkScenario() {
    std::string platform(getenv(kPlatform));

    if (StringEquals(platform, kPG)) {
        return NewPGBenchmarkScenario();
    }

    if (StringEquals(platform, kFDW)) {
        return NewFDWBenchmarkScenario();
    }

    if (StringEquals(platform, kEngine)) {
        return NewEngBenchmarkScenario();
    }

    return nullptr;
}

int main(int argc, char** argv) {
    BenchmarkScenario* s = NewBenchmarkScenario();
    if (!s) {
        std::cout << "unsupported platform!" << std::endl;
        return 0;
    }

    s->DisplayBenchmarkInfo();
    if (!s->BeforeBenchmark()) {
        std::cout << "before benchmark failed!" << std::endl;
        delete s;
        return 0;
    }

    std::string scenario(getenv(kScenario));
    if (StringEquals(scenario, kInsert)) {
        s->BenchInsertScenario();
    } else if (StringEquals(scenario, kScan)) {
        s->BenchScanScenario();
    } else {
        std::cout << "unsupported scenario!" << std::endl;
    }

    if (!s->AfterBenchmark()) {
        std::cout << "after benchmark failed!" << std::endl;
    }

    delete s;
    return 0;
}