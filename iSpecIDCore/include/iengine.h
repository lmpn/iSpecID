#ifndef IENGINE_H
#define IENGINE_H
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/regex.hpp>
#endif
#include "datatypes.h"
#include "network.h"
#include "utils.h"
#include <limits>
#include <cstdio>
#include <cfloat>
#include <stdexcept>
namespace ispecid{ 
using namespace datatypes;





using GradeFunc = std::function<std::string( Species&, Dataset&, DistanceMatrix&, GradingParameters&)>;


class IEngine
{
public:
    IEngine(int cores);
    ~IEngine(){
        task_pool->join();
        request_pool->join();
        delete task_pool;
        delete request_pool;
    };

    std::vector<std::string> annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& parametes);
    std::vector<std::string> annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& parametes);

private:
    bool speciesPerBIN(datatypes::Dataset& data, const std::string& bin);
    datatypes::Neighbour parseBoldData(std::string cluster);
    std::string findBinsNeighbour(datatypes::Dataset& data, datatypes::DistanceMatrix& distances, const std::unordered_set<std::string>& clusters, double max_distance);
    void annotateItem( datatypes::Species& species, datatypes::Dataset& data, datatypes::DistanceMatrix& distances, datatypes::GradingParameters& params);

    std::vector<std::string> errors;
    boost::asio::thread_pool* task_pool;
    boost::asio::thread_pool* request_pool;
    std::condition_variable task_cv;
    std::mutex task_lock;
    std::condition_variable request_cv;
    std::mutex request_lock;
    int completed_requests;
    int requests;
};


}
#endif // IENGINE_H
