#include "utils.h"
#include "ispecid.h"
#include <map>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;


int main(int argc, char **argv)
{
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Desktop/playground/datasets/canidae.tsv");
    std::vector<std::string> header;
    auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
    records = utils::filter(records, Record::goodRecord);
    Dataset data = utils::group(records,Record::getSpeciesName,Species::addRecord,Species::fromRecord);
    DistanceMatrix distances;
    ispecid::IEngine engine;
    GradingParameters params;
    auto errors = engine.annotate(data,distances,params);
    std::map<std::string,int> res;
    for (auto &r : data){
        auto idx = r.second.getGrade();
        res[idx] += r.second.recordCount();
    }
    PRINTV(errors);
}