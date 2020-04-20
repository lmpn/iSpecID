#include "utils.h"
#include "iengine.h"

int main(int argc, char **argv)
{
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Documents/canidae.tsv");
    IEngine engine;
    engine.load(file_path);
    engine.filter([](Record item) {return item["species_name"].empty();});
    engine.group();
    engine.annotate();
    engine.gradeRecords();
    engine.save("output.tsv");
    //PRINTV(engine.calculateGradeResults());
}
