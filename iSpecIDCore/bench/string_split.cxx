#include <benchmark/benchmark.h>
#include <string_view>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <numeric>

using namespace std::literals;

// code based on examples from https://tristanbrindle.com/posts/a-quicker-study-on-tokenising/
// and from https://marcoarena.wordpress.com/2017/01/03/string_view-odi-et-amo/

size_t numAllocations = 0;
size_t sizeAllocations = 0;


double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}



// uses string::find_first_of
std::vector<std::string>
split(const std::string& str, const std::string& delims = " ")
{
	std::vector<std::string> output;
	//output.reserve(str.length() / 4);
	size_t first = 0;

	while (first < str.size())
	{
		const auto second = str.find_first_of(delims, first);

        output.emplace_back(str.data() + first, str.data() + (second == std::string::npos ? str.size() : second));

		if (second == std::string::npos)
			break;

		first = second + 1;
	}

	return output;
}

// uses std::find_first_of
std::vector<std::string>
splitStd(const std::string& str, const std::string& delims = " ")
{
	std::vector<std::string> output;
	//output.reserve(str.length() / 4);
	auto first = std::cbegin(str);

	while (first != std::cend(str))
	{
		const auto second = std::find_first_of(first, std::cend(str),
			std::cbegin(delims), std::cend(delims));
			output.emplace_back(first, second);

		if (second == std::cend(str))
			break;

		first = std::next(second);
	}

	return output;
}

// strings, but works on pointers rather than iterators
// code by JFT
std::vector<std::string> splitPtr(const std::string& str, const std::string& delims = " ")
{
	std::vector<std::string> output;
	//output.reserve(str.size() / 2);

	for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1) {
		second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

			output.emplace_back(first, second);
	}

	return output;
}

// uses string_view::find_first_of
std::vector<std::string_view>
splitSV(std::string_view strv, std::string_view delims = " ")
{
	std::vector<std::string_view> output;
	//output.reserve(strv.length() / 4);
	size_t first = 0;

	while (first < strv.size())
	{
		const auto second = strv.find_first_of(delims, first);
		//
			output.emplace_back(strv.substr(first, second-first));

		if (second == std::string_view::npos)
			break;

		first = second + 1;
	}

	return output;
}

// uses std::find_first_of rather than string_view::find_first_of
std::vector<std::string_view>
splitSVStd(std::string_view strv, std::string_view delims = " ")
{
	std::vector<std::string_view> output;
	//output.reserve(strv.length() / 4);
	auto first = strv.begin();

	while (first != strv.end())
	{
		const auto second = std::find_first_of(first, std::cend(strv),
			std::cbegin(delims), std::cend(delims));
		//
			output.emplace_back(strv.substr(std::distance(strv.begin(), first), std::distance(first, second)));

		if (second == strv.end())
			break;

		first = std::next(second);
	}

	return output;
}

// based on the JFT's comment:
std::vector<std::string_view> splitSVPtr(std::string_view str, char delims = '\t')
{
	std::vector<std::string_view> output;
	//output.reserve(str.size() / 2);

	for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1) {
		second = std::find(first, last, delims);
        output.emplace_back(first, second - first);
	}
	return output;
}

std::vector<std::string> read(std::string arg){
    std::ifstream file(arg);
    std::vector<std::string> r;
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            r.push_back(line);
        }
        file.close();
    }
    return r;
}


static void BMsplitSVPtr(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/canidae.t");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = splitSVPtr(line,'\t');
            std::cout << result.size() << std::endl;
        }
    }
}


static void BMsplitSVStd(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = splitSVStd(line,"\t");
            
        }
    }
}

static void BMsplitSV(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = splitSV(line,"\t");
            
        }
    }
}

static void BMsplitPtr(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = splitPtr(line,"\t");
            
        }
    }
}
static void BMsplitStd(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = splitStd(line,"\t");
            
        }
    }
}
static void BMsplit(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            auto result = split(line,"\t");
            
        }
    }
}
static void BMboost(benchmark::State& state) {
    std::string arg("/Users/lmpn/Documents/dissertation/misc/datasets/tsv/culicidae.txt");
    for (auto _ : state)
    {
        auto lines = read(arg);
        for(auto& line : lines){
            std::vector<std::string> result;
            boost::split(result,line,[](char delim){return delim == '\t';});
            
        }
    }
}

BENCHMARK(BMsplitSVPtr)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(1)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(BMsplitPtr)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(BMsplitSV)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);



BENCHMARK(BMsplitSVStd)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(BMsplitStd)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(BMsplit)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(BMboost)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK_MAIN();