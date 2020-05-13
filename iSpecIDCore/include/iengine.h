#ifndef IENGINE_H
#define IENGINE_H
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "miner.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread.hpp>
#include "utils.h"
#include "species.h"
#include "record.h"

class Record;
class Species;

class IEngine
{
public:

    IEngine();
    ~IEngine(){
        pool->join();
        delete pool;
    };
    void load(std::string filePath);
    void load_distance_matrix(std::string filePath);
    void save(std::string filePath);
    void saveDistanceMatrix(std::string filePath);
    void filter(std::function<bool(const Record&)> pred);
    void group();
    void annotate(std::vector<std::string> &errors);

    std::unordered_map<std::string,int> countFilterBadEntries();
    std::vector<int> calculateGradeResults();
    void gradeRecords();


    void populateDistanceMatrix();
    inline std::vector<Record>& getEntries() { return entries;}
    inline std::vector<Record> getEntriesCopy() { return entries;}
    inline std::vector<Record> getFilteredEntriesCopy() { return filtered_entries;}
    inline void setEntries(std::vector<Record> entries) { this->entries = entries ;}
    inline void setFilteredEntries(std::vector<Record> filteredEntries) { this->filtered_entries = filteredEntries ;}
    inline std::vector<Record>& getFilteredEntries() { return filtered_entries;}
    std::unordered_map<std::string, Species> getGroupedEntries() {return grouped_entries;}
    inline int size() {return entries.size();};
    inline void setLabs(int labs){min_labs = labs >= 2 ? labs : min_labs;};
    inline void setDist(double dist){min_dist = dist <= 2 ? dist : min_dist;};
    inline void setDeposit(int deposit){min_deposit = deposit >= 3 ? deposit : min_deposit;}
    inline void clear(){
        header.clear();
        entries.clear();
        filtered_entries.clear();
        grouped_entries.clear();
    }

private:
    std::vector<std::string> header;
    std::vector<Record> entries;
    std::vector<Record> filtered_entries;
    std::unordered_map<std::string, Species> grouped_entries;
    std::unordered_map<std::string, std::pair<std::string, double>> dist_matrix;
    std::vector<std::string> errors;
    int min_labs = 2;
    double min_dist = 2;
    int min_deposit = 3;
    boost::asio::thread_pool* pool;
};

#endif // IENGINE_H
