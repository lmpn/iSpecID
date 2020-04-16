#include "annotator.h"
//#include <libxml2/libxml/tree.h>
//#include <libxml2/libxml/HTMLparser.h>


namespace annotator {

void annotationAlgo(std::unordered_map<std::string, Species>& data){
    for(auto& pair : data){
        auto& species = pair.second;
        std::string grade = "D";
        if(species.specimens.size() > 3){
            grade = "E1";
            if(species.bins.size() == 1){
                auto bin = (*species.bins.begin()).first;
                auto BINSpeciesConcordance = speciesPerBIN(data, bin);
                if(BINSpeciesConcordance){
                    grade = species.institution.size()==1 ? "B" : "A";
                }
            }else{
                grade = findBinsNeighbour(data, species.bins);
            }
            species.grade = grade;
        }else{
            species.grade = grade;
        }
    }
}



/*
     * Core Algorithm Helpers
    */

bool speciesPerBIN(std::unordered_map<std::string, Species>& data, std::string bin){
    std::unordered_set<std::string_view> unique_set;
    size_t count = 0;
    for(auto & entry : data){
        auto lst = entry.second.bins.end();
        auto result = entry.second.bins.find(bin);
        if(lst != result && (++count) == 2){
            return false;
        }
    }
    return true;
}



BoldData parseBoldData(std::string bin){
    BoldData bd;
    bd.distance = std::numeric_limits<int>::max();
    bd.neighbour = "";
    Miner mn;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        std::regex all = std::regex ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%.*</td>|Nearest BIN URI:</th>\\s*<td>(.*)</td>");
        std::string s = mn.getPage(url.c_str());
        std::smatch matches;
        std::regex_search (s,matches,all);
        auto next = matches.suffix().str();
        float d = std::stof(matches[1]);
        std::regex_search (next,matches,all);
        std::string nbin = matches[2];
        bd.distance = d;
        bd.neighbour = nbin;
    }catch (const std::exception& e) {
        //PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return bd;
}



std::string findBinsNeighbour(std::unordered_map<std::string, Species>& data, std::unordered_map<std::string, int> bins)
{
    size_t ctr, count, num_components;
    auto grade = "E2";

    ctr = 0;
    for(auto& pair : data){
        auto cur_bins = pair.second.bins;
        if(utils::hasIntersection(bins,cur_bins)){
            ctr+=1;
            if(ctr>1) return grade;
        }
    }



    std::vector<std::string> bin_names;
    for(auto& item : bins) bin_names.push_back(item.first);
    count = bin_names.size();

    ugraph graph(count);
    for(size_t  i = 0; i < count; i++){
        auto bold = parseBoldData(bin_names[i]);
        auto item = std::find(bin_names.begin(), bin_names.end(), bold.neighbour);
        size_t ind = std::distance(bin_names.begin(),item );
        if( ind >=0 && bold.distance <=2) {
            boost::add_edge(i, ind, bold.distance, graph);
        }
    }
    std::vector<int> component (count);
    num_components = boost::connected_components (graph, &component[0]);
    if( num_components == 1){
        grade="C";
    }
    return grade;
}

}

