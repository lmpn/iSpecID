#ifndef IENGINE_H
#define IENGINE_H
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread.hpp>
#include "datatypes.h"
#include "annotator.h"

namespace ispecid{ 
using namespace datatypes;





using GradeFunc = std::function<std::string( Species&, Dataset&, DistanceMatrix&, GradingParameters&)>;


class IEngine
{
public:
    IEngine(int num_cores = -1);
    ~IEngine(){
        pool->join();
        delete pool;
    };

    std::vector<std::string> annotate(Dataset& data, DistanceMatrix& distances, GradingParameters& parametes, GradeFunc grade_func = annotator::annotateItem);
    std::vector<std::string> annotate(std::vector<Record>& data, DistanceMatrix& distances, GradingParameters& parametes, GradeFunc grade_func = annotator::annotateItem);

private:
    boost::asio::thread_pool* pool;
    std::condition_variable cv;
    std::mutex lock;
};


}
#endif // IENGINE_H