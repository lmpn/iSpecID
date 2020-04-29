#include "annotator.h"
#include <boost/regex.hpp>

namespace annotator {

void annotationAlgo(std::unordered_map<std::string, Species>& data,std::vector<std::string> &errors, int min_labs, double min_dist, int min_deposit){
    /*
    Para cada especie(ESP):
        Se o #sequencias > 3:
         |   Se o #bins = 1:
         |    |   Se houver concordancia:
         |    |    |  Se o #laboratorios = 1:
         |    |    |   |   Marcar ESP com B
         |    |    |  Senão:
         |    |    |      Marcar ESP com A
         |    |   Senão:
         |    |       Marcar ESP com E1
         |   Senão:
         |       Marcar ESP com a nota dos "All the nearest neighbors"
        Senão
            Marcar ESP com D


    Para cada especie(ESP):
        Se o #laboratorios >= 2:
         |   Se o #bins = 1:
         |    |   Se houver concordancia:
         |    |    |  Se o #sequencias >= 3:
         |    |    |   |   Marcar ESP com A
         |    |    |  Senão:
         |    |    |      Marcar ESP com B
         |    |   Senão:
         |    |       Marcar ESP com E1
         |   Senão:
         |       Marcar ESP com a nota dos "All the nearest neighbors"
        Senão
            Marcar ESP com D


    */
    for(auto& pair : data){
        auto& species = pair.second;
        std::string grade = "D";
        int size = species.institution.size();
        if(size >= min_labs){

            grade = "E1";
            if(species.bins.size() == 1){
                auto bin = (*species.bins.begin()).first;
                auto BINSpeciesConcordance = speciesPerBIN(data, bin);
                if(BINSpeciesConcordance){
                    grade = species.specimens.size() >= min_deposit ? "B" : "A";
                }
            }else{
                grade = findBinsNeighbour(data, species.bins, min_dist, errors);
            }
        }
        species.grade = grade;
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




BoldData parseBoldData(std::string bin,std::vector<std::string> &errors){
    BoldData bd;
    bd.distance = std::numeric_limits<int>::max();
    bd.neighbour = "";
    Miner mn;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        std::string page = mn.getPage(url.c_str());
        /*
        std::regex all = std::regex ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%.*</td>|Nearest BIN URI:</th>\\s*<td>(.*)</td>");
        std::smatch matches;
        std::regex_search (page,matches,all);
        auto next = matches.suffix().str();
        double d = std::stod(matches[1]);
        std::regex_search (next,matches,all);
        std::string nbin = matches[2];*/
        static const boost::regex dist  ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+.\\d+)%.*</td>");
        static const boost::regex bin  ("Nearest BIN URI:</th>\\s*<td>(.*?)</td>");
        boost::smatch char_matches;
        double d;
        std::string nbin;
        if (boost::regex_search(page,char_matches, dist) )
        {
            d = std::stod(char_matches[1]);
        } 
        if (boost::regex_search(page,char_matches, bin) )
        {
            nbin = char_matches[2];
        } 
        bd.distance = d;
        bd.neighbour = nbin;
    }catch (const std::exception& e) {
        errors.push_back("Error fetching bin " + bin +" data");
    }
    return bd;
}



std::string findBinsNeighbour(std::unordered_map<std::string, Species>& data, std::unordered_map<std::string, int> bins, double min_dist,std::vector<std::string> &errors)
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
        auto bold = parseBoldData(bin_names[i],errors);
        auto item = std::find(bin_names.begin(), bin_names.end(), bold.neighbour);
        size_t ind = std::distance(bin_names.begin(),item );
        if( ind >=0 && bold.distance <= min_dist) {
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

