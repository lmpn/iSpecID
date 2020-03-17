#include <annotator.hpp>

#include <benchmark/benchmark.h>

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}
/*
static void V1_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V1::annotate(s);
}

static void V1_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V1::annotate(s);
}

static void V1_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V1::annotate(s);
}



static void V2_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V2::annotate(s);
}

static void V2_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V2::annotate(s);
}

static void V2_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V2::annotate(s);
}

static void V3_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V3::annotate(s);
}

static void V3_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V3::annotate(s);
}

static void V3_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V3::annotate(s);
}

static void V4_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V4::annotate(s);
}

static void V4_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V4::annotate(s);
}

static void V4_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V4::annotate(s);
}

static void V5_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V5::annotate(s);
}

static void V5_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V5::annotate(s);
}

static void V5_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V5::annotate(s);
}
*/


static void V6_culicidae(benchmark::State& state) {
	std::string s("/Users/lmpn/Documents/dissertation/playground/datasets/tsv/culicidae.tsv");
    for (auto _ : state)
    {
        Annotator an;
        an.annotate(s);
        auto results = an.getGradeResults();
        char g ='A';
        for(auto& i : results){
            PRINT(g++<<";"<<i.cnt<<";"<<i.perc);
        }
    };
}

static void V6_aves(benchmark::State& state) {
	std::string s("/Users/lmpn/Documents/dissertation/playground/datasets/tsv/aves.tsv");
    for (auto _ : state)
    {
        Annotator an;
        an.annotate(s);
        auto results = an.getGradeResults();
        char g ='A';
        for(auto& i : results){
            PRINT(g++<<";"<<i.cnt<<";"<<i.perc);
        }
    }
}

static void V6_canidae(benchmark::State& state) {
	std::string s("/Users/lmpn/Documents/dissertation/playground/datasets/tsv/canidae.tsv");
    for (auto _ : state)
    {
        Annotator an;
        an.annotate(s);
        auto results = an.getGradeResults();
        char g ='A';
        for(auto& i : results){
            PRINT(g++<<";"<<i.cnt<<";"<<i.perc);
        }
    }
}

/*
static void V7_culicidae(benchmark::State& state) {
	std::string s("../datasets/tsv/culicidae.tsv");
    for (auto _ : state)
        V7::annotate(s);
}

static void V7_aves(benchmark::State& state) {
	std::string s("../datasets/tsv/aves.tsv");
    for (auto _ : state)
        V7::annotate(s);
}

static void V7_canidae(benchmark::State& state) {
	std::string s("../datasets/tsv/canidae.tsv");
    for (auto _ : state)
        V7::annotate(s);
}*/

BENCHMARK(V6_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V6_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V6_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);


/*
BENCHMARK(V7_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V7_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V7_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);


BENCHMARK(V5_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V5_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V5_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(V4_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V4_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V4_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(V3_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V3_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V3_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V2_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V2_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V2_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V1_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V1_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V1_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

*/
BENCHMARK_MAIN();