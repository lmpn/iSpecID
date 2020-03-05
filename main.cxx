#include <utils.hpp>
#include <annotatev1.hpp>
#include <annotatev2.hpp>
#include <annotatev3.hpp>
#include <annotatev4.hpp>
#include <tsvreader.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>



int main(int argc, char **argv)
{
	std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Documents/dissertation/playground/datasets/tsv/canidae.tsv");
	V4::annotate(file_path);
}
