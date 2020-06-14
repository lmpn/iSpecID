#include "iengine.h"


namespace ispecid{ 
IEngine::IEngine(int cores)
{
    network::prepareNetwork();
    int available_cores = boost::thread::hardware_concurrency();
    if(cores > available_cores){
        cores = available_cores;
    }else if(cores < 2){
        throw std::runtime_error("Insuficient resources");
    }
    int task_cores = 2;
    int network_cores = cores - 2;
    if(network_cores < task_cores && cores > 2){
        network_cores = 2;
        task_cores = cores - 2;
    }else if(cores == 2){
        network_cores = 1;
        task_cores = 1;
    }
    request_pool = new boost::asio::thread_pool(network_cores);
    task_pool = new boost::asio::thread_pool(task_cores);
}


bool IEngine::speciesPerBIN(datatypes::Dataset& data, const std::string& bin){
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




datatypes::Neighbour IEngine::parseBoldData(std::string cluster){
    datatypes::Neighbour neighbour;
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



std::string IEngine::findBinsNeighbour(datatypes::Dataset& data, datatypes::DistanceMatrix& distances, const std::unordered_set<std::string>& clusters, double max_distance)
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
    bool internet = false;
    for(size_t  i = 0; i < count; i++){
        std::string bin = *cur_elem;
        auto neighbour_it = distances.find(bin);
        datatypes::Neighbour neighbour;
        if(neighbour_it != distances_end){
            neighbour = (*neighbour_it).second;
        }else{
            internet = true;
            requests++;
            boost::asio::post(*request_pool, [&,bin](){
                std::string error;
                try{
                    datatypes::Neighbour request_neighbour = parseBoldData(bin);
                    distances.insert({bin, request_neighbour});
                }catch(std::exception& e){
                    std::string exception_message(e.what());
                    error = exception_message;
                }
                {
                    auto ul = std::unique_lock<std::mutex>(task_lock);
                    errors.push_back(error);
                    completed_requests++;
                    if(completed_requests == requests){
                        request_cv.notify_one();
                    }
                }
            });
        }
        if(!internet){
            auto item = clusters.find(neighbour.clusterB);
            if( item!=bins_end && neighbour.distance <= max_distance) {
                size_t ind = std::distance(bins_begin, item);
                boost::add_edge(i, ind, neighbour.distance, graph);
            }
        }
        cur_elem++;
    }
    if(internet){
        return "Z";
    }
    std::vector<int> component (count);
    num_components = boost::connected_components (graph, &component[0]);
    if( num_components == 1){
        grade="C";
    }
    return grade;
}


void IEngine::annotateItem( datatypes::Species& species, datatypes::Dataset& data, datatypes::DistanceMatrix& distances, datatypes::GradingParameters& params){
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
            grade = findBinsNeighbour(data, distances, species.getClusters(), max_distance);
        }
    }
    species.setGrade(grade);
}

std::vector<std::string> IEngine::annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& params){
    errors.clear();
    int tasks = data.size();
    int completed = 0;
    for(auto& pair : data){
        auto& species = pair.second;
        boost::asio::post(*task_pool, [&](){
            annotateItem(species, data, distances, params);
            {
                auto ul = std::unique_lock<std::mutex>(task_lock);
                completed++;
                if(completed == tasks){
                    task_cv.notify_one();
                }
            }
        });
    }
    {
        auto ul = std::unique_lock<std::mutex>(task_lock);
        task_cv.wait(ul, [&](){return tasks == completed;});
    }
    {
        auto ul = std::unique_lock<std::mutex>(request_lock);
        request_cv.wait(ul, [&](){return requests == completed_requests;});
    }
    for(auto& pair : data){
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
