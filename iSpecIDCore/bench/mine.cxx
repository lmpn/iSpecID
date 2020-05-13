#include <iengine.h>
#include <benchmark/benchmark.h>

std::string base = "/Users/lmpn/Desktop/diss/datasets/tsv/";
int reps = 1;

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}

static void V6_culicidae(benchmark::State& state) {
    std::string s = base + "culicidae.tsv";
    std::string d = base + "dist_culicidae.csv";
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.filter([](Record item) {return item["species_name"].empty() || item["bin_uri"].empty() || item["institution_storing"].empty();});
        an.group();
        an.populateDistanceMatrix();
        an.saveDistanceMatrix("cxx_dist_culicidae.csv");
    };
}

static void V6_aves(benchmark::State& state) {
    std::string s = base + "aves.tsv";
    std::string d = base + "dist_aves.csv";
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.filter([](Record item) {return item["species_name"].empty() || item["bin_uri"].empty() || item["institution_storing"].empty();});
        an.group();
        an.populateDistanceMatrix();
        an.saveDistanceMatrix("cxx_dist_aves.csv");
    }
}

static void V6_canidae(benchmark::State& state) {
    std::string s = base + "canidae.tsv";
    std::string d = base + "dist_canidae.csv";
    for (auto _ : state)
    {
        IEngine an;
        std::vector<std::string> er;
        an.load(s);
        an.load_distance_matrix(d);
        an.filter([](Record item) {return item["species_name"].empty() || item["bin_uri"].empty() || item["institution_storing"].empty();});
        an.group();
        an.populateDistanceMatrix();
        an.saveDistanceMatrix("cxx_dist_canidae.csv");
    }
}


BENCHMARK(V6_canidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(reps)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V6_aves)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(reps)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(V6_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(reps)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);



BENCHMARK_MAIN();
