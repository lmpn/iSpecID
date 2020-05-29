#include "iengine.h"
#include "network.h"

namespace ispecid{ 
IEngine::IEngine(int num_cores)
{
    network::prepareNetwork();
    int cores = num_cores;
    int max_cores = boost::thread::hardware_concurrency();
    if(num_cores == -1 || num_cores > max_cores){
        cores = max_cores;
    }
    pool = new boost::asio::thread_pool(cores);
}

std::vector<std::string> IEngine::annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& params, GradeFunc grade_func){
    int tasks = data.size();
    std::vector<std::string> errors;
    int completed = 0;
    for(auto& pair : data){
        auto& species = pair.second;
        boost::asio::post(*pool, [&](){
            std::string error = grade_func(species, data, distances, params);
            {
                auto ul = std::unique_lock<std::mutex>(lock);
                if(!errors.empty()){
                    errors.push_back(error);
                }
                completed++;
                if(completed == tasks){
                    cv.notify_one();
                }
            }
        }
        );
    }
    {
        auto ul = std::unique_lock<std::mutex>(lock);
        cv.wait(ul, [&](){return tasks == completed;});
    }
    return errors; 
}

std::vector<std::string> IEngine::annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& params, GradeFunc grade_func){
    auto dataset = utils::group(data,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    return annotate(dataset, distances, params, grade_func); 
}

}
