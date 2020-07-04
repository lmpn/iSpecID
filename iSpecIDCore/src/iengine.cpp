#include "iengine.h"


namespace ispecid{ 
IEngine::IEngine(int cores)
{
    network::prepareNetwork();
    int available_cores = boost::thread::hardware_concurrency();
    if(cores > available_cores){
        cores = available_cores;
    }
    pool = new boost::asio::thread_pool(cores);
}


bool IEngine::speciesPerBIN(Dataset& data, const std::string& bin){
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




Neighbour IEngine::parseBoldData(std::string cluster){
    Neighbour neighbour;
    neighbour.distance = DBL_MAX;
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

std::vector<Neighbour> getNeighbours(DistanceMatrix& distances, const std::unordered_set<std::string>& clusters){
    std::vector<Neighbour> neighbours;
    auto distances_end = distances.end();
    for(auto& cluster : clusters){
        auto neighbour_it = distances.find(cluster);
        if(neighbour_it != distances_end){
            neighbours.push_back((*neighbour_it).second);
        }
    }    
    return neighbours;
}

std::string IEngine::findBinsNeighbour(Dataset& data, DistanceMatrix& distances, const std::unordered_set<std::string>& clusters, double max_distance)
{
    size_t ctr, num_components;
    auto grade = "E";

    ctr = 0;
    for(auto& pair : data){
        auto& current_clusters = pair.second.getClusters();
        if(utils::hasIntersection(clusters,current_clusters)){
            ctr+=1;
            if(ctr>1) return grade;
        }
    }


    auto cluster_begin = clusters.begin();
    auto cluster_end = clusters.end();
    auto neighbours = getNeighbours(distances, clusters);
    if(neighbours.size() != clusters.size()){
        for(auto& cluster : clusters){
            tasks++;
            boost::asio::post(*pool, [&,cluster](){
                std::string error;
                try{
                    Neighbour request_neighbour = parseBoldData(cluster);
                    distances.insert({cluster, request_neighbour});
                }catch(std::exception& e){
                    std::string exception_message(e.what());
                    error = exception_message;
                }
                {
                    auto ul = std::unique_lock<std::mutex>(task_lock);
                    if(!error.empty())
                        errors.push_back(error);
                    completed_tasks++;
                    if(completed_tasks == tasks){
                        task_cv.notify_one();
                    }
                }
            });
        }
        return "Z";
    }else{
        int i = 0;
        ugraph graph(neighbours.size());
        for(auto& neighbour : neighbours){
            auto item = clusters.find(neighbour.clusterB);
            if(item != cluster_end && neighbour.distance <= max_distance) {
                size_t ind = std::distance(cluster_begin, item);
                boost::add_edge(i, ind, neighbour.distance, graph);
            }
            i++;
        }
        std::vector<int> component (neighbours.size());
        num_components = boost::connected_components (graph, &component[0]);
        if( num_components == 1){
            grade="C";
        }
        return grade;
    }
}


void IEngine::annotateItem( Species& species, Dataset& data, DistanceMatrix& distances, GradingParameters& params){
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
                grade = specimens_size >= min_size ? "A" : "B";
            }
        }else{
            grade = findBinsNeighbour(data, distances, species.getClusters(), max_distance);
        }
    }
    species.setGrade(grade);
}

std::vector<std::string> IEngine::annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& params){
    errors.clear();
    tasks = data.size();
    completed_tasks = 0;
    for(auto& pair : data){
        auto& species = pair.second;
        annotateItem(species, data, distances, params);
        boost::asio::post(*pool, [&](){
            annotateItem(species, data, distances, params);
            {
                completed_tasks++;
                if(completed_tasks == tasks){
                    task_cv.notify_one();
                }
            }
        });
    }
    {
        auto ul = std::unique_lock<std::mutex>(task_lock);
        task_cv.wait(ul, [&](){return tasks == completed_tasks;});
    }
    for(auto& pair : data){
        auto& species = pair.second;
        if(species.getGrade() == "Z"){
            annotateItem(species, data, distances, params);
        }
    }
    return errors; 
}

std::vector<std::string> IEngine::annotateMPI(Dataset& sub_data, Dataset& data, DistanceMatrix& distances, GradingParameters& params){
    errors.clear();
    tasks = sub_data.size();
    completed_tasks = 0;
    for(auto& pair : sub_data){
        auto& species = pair.second;
        annotateItem(species, data, distances, params);
        boost::asio::post(*pool, [&](){
            annotateItem(species, data, distances, params);
            {
                completed_tasks++;
                if(completed_tasks == tasks){
                    task_cv.notify_one();
                }
            }
        });
    }
    {
        auto ul = std::unique_lock<std::mutex>(task_lock);
        task_cv.wait(ul, [&](){return tasks == completed_tasks;});
    }
    for(auto& pair : sub_data){
        auto& species = pair.second;
        if(species.getGrade() == "Z"){
            annotateItem(species, data, distances, params);
        }
    }
    return errors; 
}



std::vector<std::string> IEngine::annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& params){
    auto dataset = utils::group(data,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    return annotate(dataset, distances, params); 
}
//END ispecid
}
