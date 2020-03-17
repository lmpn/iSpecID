#ifndef ANNOTATOR_HPP
#define ANNOTATOR_HPP 
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <record.hpp>
#include <reader.hpp>
#include <species.hpp>
#include <utils.hpp>
#include <miner.hpp>
#include <regex>
#include <tuple>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


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

typedef struct bdata{
    double distance;
    std::string neighbour;
}BoldData;

typedef struct stat{
    float perc;
    int cnt;
    stat(): perc(0.0), cnt(0) {};
}Stat;


class Annotator{
    public:
    Annotator()= default;
    
    void annotate();
    void annotate(std::string file_path);
    void load(std::string file_path);
    void group();


    template<class Pred> void filter(Pred pred){
        std::vector<Record> result;
        for(auto& item : data) {
            if(!pred(item))
                result.push_back(item);
        }
        this->data.swap(result);
    }

    
    std::vector<Record> getData();
    umap<std::string, Species> getGroupData();
    std::vector<Stat> getGradeResults();
    BoldData parseBoldData(std::string bin);
    utils::Grade findBinsNeighbour(uset<std::string> bins);
    bool speciesPerBIN(std::string bin);


    private:
    std::vector<Record> data;
    umap<std::string, Species> group_data;
};
#endif 