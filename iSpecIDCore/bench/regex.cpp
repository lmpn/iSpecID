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
#include <boost/regex.hpp>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <numeric>

using namespace std::literals;
using std::string;

double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}


typedef struct bind{
    double dist;
    string bin;
}BData;

BData regex1(string text){
    std::regex all = std::regex ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%.*</td>|Nearest BIN URI:</th>\\s*<td>(.*)</td>");
    std::smatch matches;
    std::regex_search (text,matches,all);
    auto next = matches.suffix().str();
    double d = std::stod(matches[1]);
    std::regex_search (next,matches,all);
    std::string nbin = matches[2];
    BData bd;
    bd.dist = d;
    bd.bin = nbin;
    return bd;
}

BData regex2(string text){
    std::regex all = std::regex ( "Distance to Nearest Neighbor:</th>\\s*<td>(\\d+.\\d+)%.*</td>(\n.*)*Nearest BIN URI:</th>\\s*<td>(.*)</td>");
    std::smatch matches;
    std::regex_search (text,matches,all);
    auto next = matches.suffix().str();
    double d = std::stod(matches[1]);
    std::regex_search (next,matches,all);
    std::string nbin = matches[2];
    BData bd;
    bd.dist = d;
    bd.bin = nbin;
    return bd;
}

BData regex3(string text){
    //static const boost::regex
    // all( "Distance to Nearest Neighbor:</th>\\s*<td>(\\d+.\\d+)%.*</td>(\n.*)*Nearest BIN URI:</th>\\s*<td>(.*)</td>");
    static const boost::regex dist  ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+.\\d+)%.*</td>");
    static const boost::regex bin  ("Nearest BIN URI:</th>\\s*<td>(.*?)</td>");
    boost::smatch char_matches;
    double d =0.0;
    std::string nbin="";
    if (boost::regex_search(text,char_matches, dist) )
    {
        d = std::stod(char_matches[1]);
    } 
    if (boost::regex_search(text,char_matches, bin) )
    {
        nbin = char_matches[2];
    } 
    BData bd;
    bd.dist = d;
    bd.bin = nbin;
    return bd;
}
static void BMregex1(benchmark::State& state) {
    std::ifstream inFile;
    inFile.open("bold.html"); //open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    std::string text = strStream.str(); 
    for (auto _ : state)
    {
        auto o = regex1(text);
    }
}
static void BMregex2(benchmark::State& state) {
    std::ifstream inFile;
    inFile.open("bold.html"); //open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    std::string text = strStream.str(); 
    for (auto _ : state)
    {
        auto o = regex2(text);
    }
}
static void BMboost(benchmark::State& state) {
    std::ifstream inFile;
    inFile.open("bold.html"); //open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    std::string text = strStream.str(); 
    for (auto _ : state)
    {
        auto o = regex3(text);
    }
}
BENCHMARK(BMregex1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(16)
    ->Repetitions(16)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);

BENCHMARK(BMregex2)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(16)
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