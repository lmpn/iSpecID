#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <reader.hpp>
#include <tsvreader.hpp>
#include <record.hpp>
#include <algorithm>


std::vector<std::vector<std::string>> readTSV(std::string file_path, std::vector<std::string>& header)
{
    std::vector<std::vector<std::string>> data;
	TSVReader reader(file_path);
	header = reader.get_col_names();
	for (auto& row : reader)
	{
		data.push_back(row.operator std::vector<std::__cxx11::string, std::allocator<std::__cxx11::string>>());
	}
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("Number of entries read: " << data.size());
		PRINT("*--------------------------------------*");
	#endif
    return data;
}



std::vector<Record> read(std::string file_path){
    Reader<Record> r(file_path,1);
    auto recs = r.get_records();
    return recs;
}


static void MYREAD(benchmark::State& state) {
	std::string s("super");
    
    for (auto _ : state){
        auto r = read(s);
    }
        
}

static void LIBREAD(benchmark::State& state) {
	std::string s("super");
    std::vector<std::string> header;
    
    for (auto _ : state)
        auto r = readTSV(s,header);
}
double kbest(const std::vector<double>& vec){
    size_t best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    float res = 0;
    for (size_t i = 0; i < best; i++)
    {
        res = vec[i];
    }
    
    
    return res/best ;
}

BENCHMARK(MYREAD)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)->ComputeStatistics("kbest", kbest);

BENCHMARK(LIBREAD)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)->ComputeStatistics("kbest", kbest);

BENCHMARK_MAIN();