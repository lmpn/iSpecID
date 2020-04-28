#include <iengine.h>
#include <benchmark/benchmark.h>

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}

static void V6_culicidae(benchmark::State& state) {
    std::string s("/Users/lmpn/playground/dissertation/datasets/tsv/culicidae.tsv");
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.filter([](Record item) {return item["species_name"].empty();});
        an.group();
        an.annotate(er);
        an.calculateGradeResults();
    };
}

static void V6_aves(benchmark::State& state) {
    std::string s("/Users/lmpn/playground/dissertation/datasets/tsv/aves.tsv");
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.filter([](Record item) {return item["species_name"].empty();});
        an.group();
        an.annotate(er);
        an.calculateGradeResults();
    }
}

static void V6_canidae(benchmark::State& state) {
    std::string s("/Users/lmpn/playground/dissertation/datasets/tsv/canidae.tsv");
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.filter([](Record item) {return item["species_name"].empty();});
        an.group();
        an.annotate(er);
        an.calculateGradeResults();
    }
}


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



BENCHMARK_MAIN();