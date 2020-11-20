#include <iostream>
using namespace std;

#include "benchmark.h"
#include "pg_benchmark.h"
#include "fdw_benchmark.h"
#include "tbam_benchmark.h"
#include "eng_benchmark.h"
#include "util.h"

BenchmarkScenario* NewBenchmarkScenario() {
    string platform(getenv(kPlatform));

    if (StringEquals(platform, kPG)) {
        return NewPGBenchmarkScenario();
    }

    if (StringEquals(platform, kFDW)) {
        return NewFDWBenchmarkScenario();
    }

    if (StringEquals(platform, kTBAM)) {
        return NewTBAMBenchmarkScenario();
    }

    if (StringEquals(platform, kEngine)) {
        return NewEngBenchmarkScenario();
    }

    return nullptr;
}

int main(int argc, char** argv) {
    BenchmarkScenario* s = NewBenchmarkScenario();
    if (!s) {
        cout << "unsupported platform!" << endl;
        return 0;
    }

    s->DisplayBenchmarkInfo();
    if (!s->BeforeBenchmark()) {
        cout << "before benchmark failed!" << endl;
        delete s;
        return 0;
    }

    string scenario(getenv(kScenario));
    if (StringEquals(scenario, kInsert)) {
        s->BenchInsertScenario();
    } else if (StringEquals(scenario, kLoad)) {
        s->BenchLoadScenario(); 
    } else if (StringEquals(scenario, kScan)) {
        s->BenchScanScenario();
    } else if (StringEquals(scenario, kGetRandom)) {
        s->BenchGetScenario(GetType::GetRand);
    } else if (StringEquals(scenario, kGetLast)) {
        s->BenchGetScenario(GetType::GetLast);
    } else {
        cout << "unsupported scenario!" << endl;
    }

    if (!s->AfterBenchmark()) {
        cout << "after benchmark failed!" << endl;
    }

    delete s;
    return 0;
}