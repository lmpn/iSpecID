#include "utils.h"
#include "ispecid.h"
#include <map>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;


int main(int argc, char **argv)
{
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Desktop/playground/datasets/canidae.tsv");
    int threads = utils::argParse<int>(argc, argv,"--threads=", 1);
    PRINT(threads);
    std::vector<std::string> header;
    auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
    records = utils::filter(records, Record::goodRecord);
    Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    DistanceMatrix distances;
    GradingParameters params;
    ispecid::IEngine engine(threads);
    auto errors = engine.annotate(data,distances,params);
}