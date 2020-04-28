#ifndef ANNOTATOR_H
#define ANNOTATOR_H
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


#include <string>
#include "record.h"
#include "species.h"
#include "utils.h"
#include "miner.h"



namespace annotator{

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




    /*
     * Core Algorithm
    */
    void annotationAlgo(std::unordered_map<std::string, Species>& data,std::vector<std::string> &errors, int min_labs = 2, double min_dist = 2.0, int min_deposit = 3);
    /*
     * Core Algorithm Helpers
    */
    BoldData parseBoldData(std::string bin,std::vector<std::string> &errors);
    std::string findBinsNeighbour(std::unordered_map<std::string, Species>& data, std::unordered_map<std::string,int> bins, double max_dist,std::vector<std::string> &errors);
    bool speciesPerBIN(std::unordered_map<std::string, Species>& data, std::string bin);

};
#endif
