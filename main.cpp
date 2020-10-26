#include "ispecid.h"
#include "utils.h"
#include <map>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;

int main(int argc, char **argv) {
  auto t1 = std::chrono::high_resolution_clock::now();
  std::string file_path = utils::argParse<std::string>(
      argc, argv,
      "--data=", "/Users/lmpn/Desktop/diss/datasets/tsv/canidae.tsv");
  int threads = utils::argParse<int>(argc, argv, "--threads=", 1);
  std::vector<std::string> header;
  auto records = loadFile<record>(file_path, toRecord, Format::TSV);
  records = utils::filter(records, record::goodRecord);
  dataset data = utils::group(records, record::getSpeciesName,
                              species::addRecord, species::fromRecord);
  distance_matrix distances;
  grading_parameters params;
  ispecid::execution::iengine engine(threads);
  ispecid::execution::auditor auditor(params, &engine);
  auditor.execute(data);
  // auto errors = engine.annotate(data,distances,params);
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  std::cout << "Duration: " << duration << std::endl;
  for (auto &p : data) {
    PRINT(p.first << " " << p.second.getGrade());
  }
}