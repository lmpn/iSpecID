#include <utils.h>
#include <reader.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <record.h>
#include <annotator.h>



int main(int argc, char **argv)
{
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Documents/dissertation/misc/datasets/tsv/canidae.tsv");
    Annotator an;
    an.load(file_path);
    an.filter([](Record item) {return item["species_name"].empty();});
    an.group();
    an.annotationAlgo();
    an.calculateGradeResults();
    PRINTV(an.getGradeResults());
}
