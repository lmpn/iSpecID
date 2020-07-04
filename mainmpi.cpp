#include "utils.h"
#include "ispecid.h"
#include <map>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <cstring>

using namespace ispecid::datatypes;
using namespace ispecid::fileio;

int main(int argc, char** argv)
{
    std::string file_path = utils::argParse<std::string>(argc, argv, "--data=", "/Users/lmpn/Desktop/diss/datasets/tsv/canidae.tsv");
    int threads = utils::argParse<int>(argc, argv, "--threads=", 1);
    std::vector<std::string> header;
    auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
    records = utils::filter(records, Record::goodRecord);
    Dataset data = utils::group(records, Record::getSpeciesName, Species::addRecord, Species::fromRecord);
    DistanceMatrix distances;
    GradingParameters params;
    // ispecid::IEngine engine(threads);
    // auto errors = engine.annotateOmp(data,distances,params);
    int ierr;
    int num_procs;
    int rank;
    ierr = MPI_Init(&argc, &argv);

    if (ierr != 0) {
        std::cout << "\n";
        std::cout << "ISID - Fatal error!\n";
        std::cout << "  MPI_Init returned ierr = " << ierr << "\n";
        exit(1);
    }
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    if (rank == 0) {
        std::cout << "\n";
        std::cout << "ISID:\n";
        std::cout << "  The number of processes available is " << num_procs << "\n";
        std::cout << "  Data size: " << data.size() << "\n";
    }
    int data_size = data.size();
    int distances_size = distances.size();
    double params_dist = params.max_distance;
    int params_size = params.min_size;
    int params_sources = params.min_sources;
    MPI_Bcast(&threads, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&distances_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params_dist, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params_sources, 1, MPI_INT, 0, MPI_COMM_WORLD);
    std::cout << "rank: " << rank << "\n"
        << "threads: " << threads << "\n"
        << "data_size: " << data_size << "\n"
        << "distances_size: " << distances_size << "\n"
        << "params_size: " << params_size << "\n"
        << "params_dist: " << params_dist << "\n"
        << "params_sources: " << params_sources << "\n"
        << std::endl;
    GradingParameters local_params = { params_sources, params_size, params_dist };
    Dataset local_data;
    DistanceMatrix local_distances;

    Dataset::iterator it;
    if (rank == 0) {
        it = data.begin();
    }

    //SEND DATASET
    for (int i = 0; i < data_size; i++) {
        int cluster_size;
        int sources_size;
        int record_count;
        char* species_name;
        int species_name_size;
        char* grade;
        int grade_size;
        std::unordered_set<std::string>::iterator clusters_it;
        std::unordered_set<std::string>::iterator sources_it;
        std::unordered_set<std::string> clusters;
        std::unordered_set<std::string> sources;
        if (rank == 0) {
            cluster_size = it->second.clustersCount();
            sources_size = it->second.sourcesCount();
            record_count = it->second.recordCount();
            species_name = strdup(it->second.getSpeciesName().data());
            species_name_size = strlen(species_name)+1;
            grade = strdup(it->second.getGrade().data());
            grade_size = strlen(grade)+1;
            clusters_it = it->second.getClusters().begin();
            sources_it = it->second.getSources().begin();
        }
        MPI_Bcast(&cluster_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&sources_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&record_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&species_name_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(rank != 0){
            species_name = new char[species_name_size];
        }
        MPI_Bcast(species_name, species_name_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Bcast(&grade_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(rank != 0){
            grade = new char[grade_size];
        }
        MPI_Bcast(grade, grade_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        std::cout << "rank: " << rank << "\n"
            << "clusters: " << cluster_size << "\n"
            << "sources: " << sources_size << "\n"
            << "records: " << record_count << "\n"
            << "species name: " << species_name << "\n"
            << "grade: " << grade << "\n"
            << std::endl; 
        for (int cs = 0; cs < cluster_size; cs++) {
            int size;
            char* buf;
            if (rank == 0) {
                size = clusters_it->size()+1;
                buf = strdup(clusters_it->data());
            }
            MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(rank!=0){
                buf = new char[size];
            }
            MPI_Bcast(buf, size + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
            clusters.insert(std::string(buf));
            if (rank == 0) {
                clusters_it++;
            }
        }
        for (int ss = 0; ss < sources_size; ss++) {
            int size;
            char* buf;
            if (rank == 0) {
                size = sources_it->size()+1;
                buf = strdup(sources_it->data());
            }
            MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(rank!=0){
                buf = new char[size];
            }
            MPI_Bcast(buf, size + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
            sources.insert(std::string(buf));
            if (rank == 0){
                sources_it++;
            }
        }
        Species nsp = {std::string(species_name), std::string(grade)};
        nsp.setClusters(clusters);
        nsp.setSources(sources);
        nsp.setRecordCount(record_count);
        local_data.insert({std::string(species_name), nsp });
        if (rank == 0)
        {
          it++;
        }
    }



    //

    MPI_Finalize();
    return 0;
}
