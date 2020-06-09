#ifndef ANNOTATOR_H
#define ANNOTATOR_H
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#ifndef Q_MOC_RUN
#include <boost/regex.hpp>
#endif
#include <limits>
#include "network.h"
#include "datatypes.h"
#include "utils.h"



namespace ispecid{ namespace annotator{

    

    /*
     * Core Algorithm
    */
    std::vector<std::string> annotationAlgo(
            datatypes::Dataset& data,
            datatypes::DistanceMatrix& distances,
            datatypes::GradingParameters& params
        );

    std::string annotateItem(
            datatypes::Species& species, 
            datatypes::Dataset& data,
            datatypes::DistanceMatrix& dist_matrix,
            datatypes::GradingParameters& params
        );
}}
#endif
