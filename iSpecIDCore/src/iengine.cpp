#include "iengine.h"

namespace ispecid{ 
IEngine::IEngine()
{

    int cores = boost::thread::hardware_concurrency();
    pool = new boost::asio::thread_pool(cores);
}

std::vector<std::string> IEngine::annotate(datatypes::Dataset& data, datatypes::DistanceMatrix& distances, datatypes::GradingParameters& params, GradeFunc grade_func){
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

}