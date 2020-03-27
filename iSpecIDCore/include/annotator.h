#ifndef ANNOTATOR_H
#define ANNOTATOR_H
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <record.h>
#include <reader.h>
#include <species.h>
#include <utils.h>
#include <miner.h>
#include <regex>
#include <tuple>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <algorithm>


template <typename T>
using matrix = std::vector<std::vector<T>>;

template <typename T>
using uset = std::unordered_set<T>;

template<typename K, typename V>
using umap = std::unordered_map<K,V>;

typedef boost::adjacency_list<
    boost::listS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, double>
> ugraph;

typedef struct brecords{
    double distance;
    std::string neighbour;
}BoldData;


class Annotator{
public:
    Annotator(){
    };

    /*
     * Core Algorithm
    */
    void annotationAlgo();

    /*
     * Post-processing functions
    */
    void gradeRecords();
    void calculateGradeResults();


    /*
     * Pre-processing functions
    */
    void load(std::string file_path);
    void cleanLoad(std::string file_path);
    void clearGroup();
    void group();
    void clear();
    template<class Pred> void filter(Pred pred){
        std::vector<Record> result;
        for(auto& item : records) {
            if(!pred(item))
                result.push_back(item);
        }
        this->records.swap(result);
    }
    template<class Pred> void filter(std::vector<Pred> preds, bool all = false){
        std::vector<Record> result;
        if(all){
            auto p = [preds](Record item){
                return std::all_of(
                            preds.begin(),
                            preds.end(),
                            [item](Pred pred){
                                return pred(item);
                            }
                );
            };
            filter(p);
        }else{
            auto p = [preds](Record item){
                return std::any_of(
                            preds.begin(),
                            preds.end(),
                            [item](Pred pred){
                                return pred(item);
                            }
                );
            };
            filter(p);
        }
    }


    /*
     * Getters
    */
    std::vector<int> getGradeResults();
    std::vector<Record> getRecords();
    umap<std::string, Species> getGroupRecords();

    /*
     * Core Algorithm Helpers
    */
    BoldData parseBoldData(std::string bin);
    std::string findBinsNeighbour(uset<std::string> bins);
    bool speciesPerBIN(std::string bin);

    int size() { return records.size();}
    void setRecords(std::vector<Record> records) { this->records = records;}

private:
    std::vector<int> results;
    std::vector<Record> records;
    umap<std::string, Species> group_records;
    std::regex dist= std::regex("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%");
    std::regex nearest = std::regex ("Nearest BIN URI:</th>\\s*<td>(.*)</td>");
};
#endif
