#include <iengine.h>
#include <benchmark/benchmark.h>

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}


inline std::string g(Record r) {
    /**
     * 
     * ops 
    */
    return r["species_name"];
};

inline void j(Species& species, Record record){
    species.push_back(record);
};

static void an_group(benchmark::State& state) {
    std::string s("culicidae.tsv");
    IEngine an;
    an.load(s);
    an.filter([](Record item) {return item["species_name"].empty();});
    for (auto _ : state)
    {
        an.group();
        auto x = an.getGroupedEntries();
    };
}

BENCHMARK(an_group)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(5)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK_MAIN();