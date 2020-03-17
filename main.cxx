#include <utils.hpp>
#include <reader.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <record.hpp>
#include <annotator.hpp>



int main(int argc, char **argv)
{
	std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Documents/dissertation/playground/datasets/tsv/canidae.tsv");
    Annotator an;
    an.annotate(file_path);
    auto results = an.getGradeResults();
    char g ='A';
    for(auto& i : results){
        PRINT(g++<<";"<<i.cnt<<";"<<i.perc);
    }
}
