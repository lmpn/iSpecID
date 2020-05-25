#include "utils.h"
#include "ispecid.h"
#include <map>
#include <benchmark/benchmark.h>

std::string base = "/Users/lmpn/Desktop/diss/datasets/tsv/";
int reps = 1;
using namespace ispecid::datatypes;
using namespace ispecid::fileio;

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}

static void V6_culicidae(benchmark::State& state) {
    std::string file_path = base + "culicidae.tsv";
    std::string d = base + "dist_culicidae.csv";
    for (auto _ : state)
    {
        auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
        records = utils::filter(records, Record::goodRecord);
        Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
        DistanceMatrix distances;
        ispecid::IEngine engine;
        // /home/a77763t/dissertation/playground/datasets
        GradingParameters params;
        engine.annotate(data,distances,params);
        std::map<std::string,int> res;
        for (auto &r : data){
            auto idx = r.second.getGrade();
            res[idx] += r.second.recordCount();
        }
    };
}

static void V6_aves(benchmark::State& state) {
    std::string file_path = base + "aves.tsv";
    std::string d = base + "dist_aves.csv";
    for (auto _ : state)
    {
        auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
        records = utils::filter(records, Record::goodRecord);
        Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
        DistanceMatrix distances;
        ispecid::IEngine engine;
        GradingParameters params;
        engine.annotate(data,distances,params);
        std::map<std::string,int> res;
        for (auto &r : data){
            auto idx = r.second.getGrade();
            res[idx] += r.second.recordCount();
        };
    }
}

static void V6_canidae(benchmark::State& state) {
    std::string file_path = base + "canidae.tsv";
    std::string d = base + "dist_canidae.csv";
    for (auto _ : state)
    {
        auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
        records = utils::filter(records, Record::goodRecord);
        Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
        DistanceMatrix distances;
        ispecid::IEngine engine;
        GradingParameters params;
        engine.annotate(data,distances,params);
        std::map<std::string,int> res;
        for (auto &r : data){
            auto idx = r.second.getGrade();
            res[idx] += r.second.recordCount();
        }
    }
}

BENCHMARK(V6_culicidae)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(reps)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);


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


BENCHMARK_MAIN();
