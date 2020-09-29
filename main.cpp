#include "utils.h"
#include "ispecid.h"
#include <map>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;


int main(int argc, char **argv)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Desktop/diss/datasets/tsv/canidae.tsv");
    int threads = utils::argParse<int>(argc, argv,"--threads=", 1);
    std::vector<std::string> header;
    auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
    records = utils::filter(records, Record::goodRecord);
    Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    DistanceMatrix distances;
    GradingParameters params;
    ispecid::IEngine engine(threads);
    auto errors = engine.annotate(data,distances,params);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << "Duration: " <<duration <<std::endl;
}