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
#include <atomic>
#include <functional>
#include <stdexcept>
namespace ispecid{ 
using namespace datatypes;





using GradeFunc = std::function<std::string( Species&, Dataset&, DistanceMatrix&, GradingParameters&)>;


class IEngine
{
public:
    IEngine(int cores, int processes = 1);
    ~IEngine(){
        pool->join();
        delete pool;
    };

    std::vector<std::string> new_annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& parametes);
    std::vector<std::string> annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& parametes);
    std::vector<std::string> annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& parametes);
    std::vector<std::string> annotateMPI(Dataset& sub_data, Dataset& data, DistanceMatrix& distances, GradingParameters& params);

private:
    bool speciesPerBIN(Dataset& data, const std::string& bin);
    Neighbour parseBoldData(std::string cluster);
    std::string findBinsNeighbour(Species& species, Dataset& data, DistanceMatrix& distances, double max_distance);
    void annotateItem( Species& species, Dataset& data, DistanceMatrix& distances, GradingParameters& params);
    void annotateItems(std::vector<Species>* species, Dataset data, DistanceMatrix distances, GradingParameters params);
    std::vector<std::vector<Species>*> divide(Dataset& data);
    std::vector<std::vector<std::string>*> divide_neighbours(std::vector<std::string> data);
    void fetchData(std::vector<std::string>* clusters, DistanceMatrix* distances);
    void wait();
    void createWork(std::vector<std::vector<Species>*>& partitions, Dataset& data, DistanceMatrix& distances, GradingParameters& params);
    void createFetch(std::vector<std::vector<std::string>*>& clusters_partitions, DistanceMatrix* distances_ptr);

    std::vector<std::string> errors;
    boost::asio::thread_pool* pool;
    std::condition_variable task_cv;
    std::mutex task_lock;
    std::atomic<int> completed_tasks;
    std::atomic<int> tasks;
    int _cores;
};


}
#endif // IENGINE_H
