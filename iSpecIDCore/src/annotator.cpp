#include "annotator.h"

namespace ispecid{namespace annotator {



//   std::regex all = std::regex ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%.*</td>|Nearest BIN URI:</th>\\s*<td>(.*)</td>");
//         std::string s = mn.getPage(url.c_str());
//         std::smatch matches;
//         std::regex_search (s,matches,all);
//         auto next = matches.suffix().str();
//         float d = std::stof(matches[1]);
//         std::regex_search (next,matches,all);
//         std::string nbin = matches[2];
//         bd.distance = d;
//         bd.neighbour = nbin;
/*
     * Core Algorithm Helpers
    */

bool speciesPerBIN(datatypes::Dataset& data, const std::string& bin){
    std::unordered_set<std::string_view> unique_set;
    size_t count = 0;
    for(auto & entry : data){
        auto lst = entry.second.getClusters().end();
        auto result = entry.second.getClusters().find(bin);
        if(lst != result && (++count) == 2){
            return false;
        }
    }
    return true;
}




datatypes::Neighbour parseBoldData(std::string cluster){
    datatypes::Neighbour neighbour;
    neighbour.distance = std::numeric_limits<int>::max();
    neighbour.clusterA = cluster; 
    neighbour.clusterB = "";
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(cluster));

    std::string page = network::getPage(url.c_str());
    static const boost::regex distance_regex  ("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+.\\d+)%.*</td>");
    static const boost::regex cluster_regex  ("Nearest BIN URI:</th>\\s*<td>(.*?)</td>");
    boost::smatch char_matches;
    if (boost::regex_search(page,char_matches, distance_regex) )
    {
        neighbour.distance = std::stod(char_matches[1]);
    }else{
        throw std::runtime_error("Distance not found when retrieving " + cluster);
    }
    if (boost::regex_search(page,char_matches, cluster_regex) )
    {
        neighbour.clusterB = char_matches[1];
    }else{
        throw std::runtime_error("Neighbour not found when retrieving " + cluster);
    }
    return neighbour;
}



std::string findBinsNeighbour(datatypes::Dataset& data, datatypes::DistanceMatrix& distances, const std::unordered_set<std::string>& clusters, double max_distance, std::string &error)
{
    size_t ctr, count, num_components;
    auto grade = "E";

    ctr = 0;
    for(auto& pair : data){
        auto& current_clusters = pair.second.getClusters();
        if(utils::hasIntersection(clusters,current_clusters)){
            ctr+=1;
            if(ctr>1) return grade;
        }
    }
    count = clusters.size();
    auto bins_begin = clusters.begin();
    auto cur_elem = clusters.begin();
    auto bins_end = clusters.end();
    auto distances_end = distances.end();
    datatypes::ugraph graph(count);
    for(size_t  i = 0; i < count; i++){
        auto bin = *cur_elem;
        auto neighbour_it = distances.find(bin);
        datatypes::Neighbour neighbour;
        if(neighbour_it != distances_end){
            neighbour = (*neighbour_it).second;
        }else{
            try{
                neighbour = parseBoldData(bin);
                distances.insert({bin, neighbour});
            }catch(std::exception& e){
                std::string exception_message(e.what());
                error = exception_message;
            }
        }
        auto item = clusters.find(neighbour.clusterB);
        if( item!=bins_end && neighbour.distance <= max_distance) {
            size_t ind = std::distance(bins_begin, item);
            boost::add_edge(i, ind, neighbour.distance, graph);
        }
        cur_elem++;
    }
    std::vector<int> component (count);
    num_components = boost::connected_components (graph, &component[0]);
    if( num_components == 1){
        grade="C";
    }
    return grade;
}


std::string annotateItem( datatypes::Species& species, datatypes::Dataset& data, datatypes::DistanceMatrix& distances, datatypes::GradingParameters& params){
    std::string error;
    int min_sources = params.min_sources;
    int min_size = params.min_size;
    double max_distance = params.max_distance;
    
    std::string grade = "D";
    int size = species.sourcesCount();
    if(size >= min_sources){

        grade = "E";
        if(species.clustersCount() == 1){
            const std::string& bin = species.getFirstCluster();
            auto BINSpeciesConcordance = speciesPerBIN(data, bin);
            if(BINSpeciesConcordance){
                int specimens_size = species.recordCount();
                grade = specimens_size >= min_size ? "B" : "A";
            }
        }else{
            grade = findBinsNeighbour(data, distances, species.getClusters(), max_distance, error);
        }
    }
    species.setGrade(grade);
    return error;
}

 std::vector<std::string> annotationAlgo(
            datatypes::Dataset& data,
            datatypes::DistanceMatrix& distances,
             datatypes::GradingParameters& params
        ){
    std::vector<std::string> errors;
    for(auto& pair : data){
        auto& species = pair.second;
        auto error = annotateItem(species, data, distances, params);
        errors.push_back(error);

        /*
        auto& species = pair.second;
        int size = species.institution.size();
        if(size >= min_labs){
            grade = "E";
            if(species.bins.size() == 1){
                const std::string& bin = (*species.bins.begin()).first;
                auto BINSpeciesConcordance = speciesPerBIN(data, bin);
                if(BINSpeciesConcordance){
                    int specimens_size = species.specimens_size;
                    grade = specimens_size >= min_deposit ? "B" : "A";
                }
            }else{
                grade = findBinsNeighbour(data, species.bins, min_dist, errors);
            }
        }
        species.grade = grade;
        grade = "D";
        */
    }
    return errors;
}
}}