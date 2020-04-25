#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/HTMLparser.h>
#include <benchmark/benchmark.h>
#include <regex>
#include "miner.h"
#include "annotator.h"


double kbest(const std::vector<double>& vec){
    int best = vec.size() * 0.625;
    std::vector<double> cvec(vec);
    std::sort(cvec.begin(),cvec.end());
    double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
    return result;
}

int traverse_dom_trees(xmlNode * a_node, annotator::BoldData& bd)
{
    xmlNode *cur_node = NULL;
    int found = 0;
    if(NULL == a_node)
    {
        //printf("Invalid argument a_node %p\n", a_node);
        return 0;
    }
    for (cur_node = a_node; cur_node && found == 0; cur_node = cur_node->next)
    {
        if(cur_node->type == XML_TEXT_NODE)
        {
            if(!strcmp( (char *)cur_node->content, "Distance to Nearest Neighbor:") ){
                return 1;
            }
            if(!strcmp( (char *)cur_node->content, "Nearest BIN URI:") ){
                return 2;
            }
        }
        found = traverse_dom_trees(cur_node->children, bd);

    }
    if(found ==3){
            return 3;
        }
        else if(found == 1){
            bd.distance = atof((char*)cur_node->next->children->content);
        }else if(found == 2 ){
            bd.neighbour = std::string((char*)cur_node->next->children->content);
            return 3;
        }

    return 0;
}

annotator::BoldData parseBoldDataXML(std::string bin){
    annotator::BoldData bd;
    bd.distance = std::numeric_limits<int>::max();
    bd.neighbour = "";
    Miner mn;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        std::string page = mn.getPage(url.c_str());
        if(page.empty()){
            PRINT("Error curl");
            return bd;
        }
        const xmlChar * content = ((const xmlChar*) page.data());
        auto doc = htmlReadDoc(content,url.data(),"UTF-8", HTML_PARSE_NOBLANKS|HTML_PARSE_NOIMPLIED|HTML_PARSE_NOERROR);
        if (doc == NULL)
        {
            return bd;
        }

        auto root_element = xmlDocGetRootElement(doc);

        if (root_element == NULL)
        {
            xmlFreeDoc(doc);
            return bd;
        }
        traverse_dom_trees(root_element, bd);
        xmlFreeDoc(doc);       // free document
        xmlCleanupParser();    // Free globals
    }catch (const std::exception& e) {
        PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return bd;
}



annotator::BoldData parseBoldDataRegex(std::string bin){
    annotator::BoldData bd;
    bd.distance = std::numeric_limits<int>::max();
    bd.neighbour = "";
    Miner mn;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        std::regex all = std::regex ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%.*</td>|Nearest BIN URI:</th>\\s*<td>(.*)</td>");
        std::string page = mn.getPage(url.c_str());
        std::smatch matches;
        std::regex_search (page,matches,all);
        auto next = matches.suffix().str();
        float d = std::stof(matches[1]);
        std::regex_search (next,matches,all);
        std::string nbin = matches[2];
        bd.distance = d;
        bd.neighbour = nbin;
    }catch (const std::exception& e) {
        PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return bd;
}

static void BMparseBoldDataRegex(benchmark::State& state) {
    std::string s("BOLD:AAC6751");
    for (auto _ : state)
    {
        auto bd = parseBoldDataRegex(s);
        PRINT(bd.distance << " " << bd.neighbour);
    };
}

static void BMparseBoldDataXML(benchmark::State& state) {
    std::string s("BOLD:AAC6751");
    for (auto _ : state)
    {
        auto bd = parseBoldDataXML(s);
        PRINT(bd.distance << " " << bd.neighbour);
    };
}
std::string getbold(std::string bin){
    Miner mn;
    std::string page;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        page= mn.getPage(url.c_str());
    }catch (const std::exception& e) {
        PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return page;
}
static void BMgetbold(benchmark::State& state) {
    std::string s("BOLD:AAC6751");
    for (auto _ : state)
    {
        auto bd = getbold(s);
        auto i = bd.size();
    };
}

BENCHMARK(BMgetbold)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(8)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(BMparseBoldDataXML)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(8)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);
BENCHMARK(BMparseBoldDataRegex)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(8)
    ->Repetitions(8)
    ->ReportAggregatesOnly(true)
    ->ComputeStatistics("kbest", kbest);



BENCHMARK_MAIN();