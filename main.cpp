#include "utils.h"
#include "parser.h"
#include "iengine.h"
#include "miner.h"
#include "annotator.h"

void run(int argc, char **argv){
    std::string file_path = utils::argParse<std::string>(argc, argv,"--data=", "/Users/lmpn/Desktop/playground/datasets/canidae.tsv");
    PRINT(file_path);
    IEngine engine;
    std::vector<std::string> er;
    engine.load(file_path);
    engine.load_distance_matrix("example.csv");
    engine.filter([](Record item) {return item["species_name"].empty() || item["bin_uri"].empty() || item["institution_storing"].empty();});
    engine.group();
    engine.annotate(er);
    engine.gradeRecords();
    PRINTV(engine.calculateGradeResults());
}

int main(int argc, char **argv)
{
    parse::Parser p(parse::Format::TSV);
    parse::InMapper<std::string> mapper = [](std::vector<std::string> h, csv::CSVRow& row){
        return row[0];
    };
    parse::OutMapper<std::string> out = [](std::string row){
        std::vector<std::string> r = {row};
        return r;
    };
    auto rows = p.loadFile("/Users/lmpn/Desktop/playground/datasets/canidae.tsv");
    auto h = p.getHeader();
    p.saveFile("c.csv",h,rows,parse::Format::TSV);
}
//0 4 33 179 717 933 
