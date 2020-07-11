#include "utils.h"
#include "ispecid.h"
#include <map>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;


int main(int argc, char **argv)
{
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
    int size = 0;
    for(auto& pair: data){
        auto s = pair.second;
        std::cout << s.getSpeciesName() << " " << s.getGrade() << std::endl;
    }
}