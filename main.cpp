#include "utils.h"
#include "iengine.h"
#include "miner.h"
#include "annotator.h"

void run(int argc, char **argv){
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/playground/datasets/canidae.tsv");
    IEngine engine;
    std::vector<std::string> er;
    engine.load(file_path);
    engine.filter([](Record item) {return item["species_name"].empty() || item["bin_uri"].empty() || item["institution_storing"].empty();});
    engine.group();
    engine.annotate(er);
    engine.gradeRecords();
    PRINTV(engine.calculateGradeResults());
}

int main(int argc, char **argv)
{
   run(argc,argv); 

}
