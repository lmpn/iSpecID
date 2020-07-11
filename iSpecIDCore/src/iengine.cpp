#include "iengine.h"


namespace ispecid{ 
IEngine::IEngine(int cores, int processes)
{
    _cores = cores/processes;
    network::prepareNetwork();
    int available_cores = boost::thread::hardware_concurrency();
    if(_cores > available_cores){
        _cores = available_cores;
    }
    pool = new boost::asio::thread_pool(_cores);
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

std::string IEngine::findBinsNeighbour(Species& species, Dataset& data, DistanceMatrix& distances, double max_distance)
{
    size_t ctr, num_components;
    auto grade = "E";
    const std::unordered_set<std::string> clusters = species.getClusters();
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
        return "U";
    }else{
        int i = 0;
        ugraph graph(neighbours.size());
        for(auto& neighbour : neighbours){
            auto item = clusters.find(neighbour.clusterA);
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
            grade = findBinsNeighbour(species, data, distances, max_distance);
        }
    }
    species.setGrade(grade);
}



void IEngine::annotateItems(std::vector<Species>* species, Dataset data, DistanceMatrix distances, GradingParameters params){
    for (auto& sp : *species)
    {
        annotateItem(sp, data, distances, params);
    }
}

void IEngine::fetchData(std::vector<std::string>* clusters, DistanceMatrix* distances){
    for (auto& cluster : *clusters)
    {
        try{
            Neighbour request_neighbour = parseBoldData(cluster);
            distances->insert({cluster, request_neighbour});
        }catch(std::exception e){
            errors.push_back(e.what());
        }
    }
}


template<class T>
std::vector<std::vector<T>*> divide_(std::vector<T> data, int divisions){
    std::vector<std::vector<T>*> partitions;
    int current_index = 0;
    for(int i = 0; i < divisions; i++){
        partitions.push_back(new std::vector<T>);
    }
    for(auto& item : data){
        partitions[current_index]->push_back(item);
        current_index = (current_index + 1) % divisions;
    }
    return partitions;
}


std::vector<std::vector<Species>*> IEngine::divide(Dataset& data){
    std::vector<std::vector<Species>*> partitions;
    for(int i = 0; i < _cores; i++){
        partitions.push_back(new std::vector<Species>);
    }
    int current_index = 0;
    for(auto& item : data){
        if(item.second.getGrade() == "U")
        {
            partitions[current_index]->push_back(item.second);
            current_index = (current_index + 1) % _cores;
        }
    }
    return partitions;
}

void IEngine::wait(){
    auto ul = std::unique_lock<std::mutex>(task_lock);
    task_cv.wait(ul, [this](){return tasks == completed_tasks;});
}


void IEngine::createFetch(std::vector<std::vector<std::string>*>& clusters_partitions, DistanceMatrix* distances_ptr){
    for(auto part : clusters_partitions){
        boost::asio::post(*pool, [this, part, distances_ptr](){
            fetchData(part, distances_ptr);
            {
                completed_tasks++;
                if(completed_tasks == tasks){
                    task_cv.notify_one();
                }
            }
        });
    }
}

void IEngine::createWork(std::vector<std::vector<Species>*>& partitions, Dataset& data, DistanceMatrix& distances, GradingParameters& params){
    for(auto part : partitions){
        boost::asio::post(*pool, [this, part, data, distances, params](){
            annotateItems(part, data, distances, params);
            {
                completed_tasks++;
                if(completed_tasks == tasks){
                    task_cv.notify_one();
                }
            }
        });
    }
}

std::vector<std::string> IEngine::annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& params){
    errors.clear();
    completed_tasks = 0;
    tasks = _cores;
    std::vector<std::vector<Species>*> partitions = divide(data);
    DistanceMatrix* distances_ptr = &distances;
    std::vector<std::string> clusters;
    std::vector<Species> z_species;
    std::vector<std::vector<std::string>*> clusters_partitions;
    std::vector<std::vector<Species>*> z_partitions;

    createWork(partitions, data, distances, params);
    wait();


    for(auto& part : partitions){
        for(auto& sp : *part){
            if(sp.getGrade() == "U"){
                auto input = sp.getClusters();
                std::copy(input.begin(), input.end(), std::back_inserter(clusters));
                z_species.push_back(sp);
            }else{
                data[sp.getSpeciesName()].setGrade(sp.getGrade());
            }
        }
    }


    completed_tasks = 0;
    z_partitions = divide_(z_species, _cores);
    clusters_partitions = divide_<std::string>(clusters, _cores);
    createFetch(clusters_partitions, distances_ptr);
    wait();
    completed_tasks = 0;
    createWork(z_partitions, data, distances, params);
    wait();
    for(auto& part : z_partitions){
        for(auto& sp : *part){
            data[sp.getSpeciesName()].setGrade(sp.getGrade());
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
    {
        auto ul = std::unique_lock<std::mutex>(task_lock);
        task_cv.wait(ul, [&](){return tasks == completed_tasks;});
    }
    return errors; 
}



std::vector<std::string> IEngine::annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& params){
    auto dataset = utils::group(data,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    return annotate(dataset, distances, params); 
}
//END ispecid
}