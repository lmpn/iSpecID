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

void masterSetup(
    int argc, 
    char** argv,
    int rank, 
    int& threads, 
    Dataset& data, 
    DistanceMatrix& distances,
    GradingParameters& params,
    int& data_size, 
    int& distances_size, 
    double& params_dist, 
    int& params_size, 
    int& params_sources, 
    int num_procs)
{
    if (rank == 0) {
        std::string file_path = utils::argParse<std::string>(argc, argv, "--data=", "/Users/lmpn/Desktop/diss/datasets/tsv/canidae.tsv");
        threads = utils::argParse<int>(argc, argv, "--threads=", 1);
        auto records = loadFile<Record>(file_path, toRecord, Format::TSV);
        records = utils::filter(records, Record::goodRecord);
        data = utils::group(records, Record::getSpeciesName, Species::addRecord, Species::fromRecord);
        data_size = data.size();
        distances_size = distances.size();
        params_dist = params.max_distance;
        params_size = params.min_size;
        params_sources = params.min_sources;
        std::cout << "\n";
        std::cout << "ISID:\n";
        std::cout << "  The number of processes available is " << num_procs << "\n";
        std::cout << "  Data size: " << data.size() << "\n";
    }
}

void broadcastSetup(int& threads, int& data_size, int& distances_size, double& params_dist, int& params_size, int& params_sources){
    MPI_Bcast(static_cast<void *>( &threads), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(static_cast<void *>( &data_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(static_cast<void *>( &distances_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(static_cast<void *>( &params_dist), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(static_cast<void *>( &params_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(static_cast<void *>( &params_sources), 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void broadcastDataset(int rank, int data_size, Dataset& data){
    Dataset::iterator it;
    if (rank == 0) {
        it = data.begin();
    }
    for (int i = 0; i < data_size; i++) {
        int cluster_size;
        int sources_size;
        int record_count;
        char* species_name;
        int species_name_size;
        char* grade;
        int grade_size;
        std::unordered_set<std::string>::const_iterator clusters_it;
        std::unordered_set<std::string>::const_iterator sources_it;
        std::unordered_set<std::string> clusters;
        std::unordered_set<std::string> sources;
        if (rank == 0) {
            cluster_size = it->second.clustersCount();
            sources_size = it->second.sourcesCount();
            record_count = it->second.recordCount();
            species_name = strdup(it->second.getSpeciesName().c_str());
            species_name_size = it->second.getSpeciesName().length();
            grade = strdup(it->second.getGrade().c_str());
            grade_size = it->second.getGrade().length();
            clusters_it = it->second.getClusters().begin();
            sources_it = it->second.getSources().begin();
        }
        MPI_Bcast(static_cast<void *>( &cluster_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(static_cast<void *>( &sources_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(static_cast<void *>( &record_count), 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(static_cast<void *>( &species_name_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(rank != 0){
            species_name = static_cast<char*>(malloc(sizeof(char)*species_name_size));
        }
        MPI_Bcast(static_cast<void *> (species_name), species_name_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Bcast(static_cast<void *>( &grade_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(rank != 0){
            grade = static_cast<char*>(malloc(sizeof(char)*grade_size));
        }
        MPI_Bcast(static_cast<void *> (grade), grade_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        for (int cs = 0; cs < cluster_size; cs++) {
            int size;
            char* buf;
            if (rank == 0) {
                size = clusters_it->length();
                buf = strdup(clusters_it->c_str());
            }
            MPI_Bcast(static_cast<void *>( &size), 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(rank!=0){
                buf = static_cast<char*>(malloc(sizeof(char)*size));
            }
            MPI_Bcast(static_cast<void *> (buf), size, MPI_CHAR, 0, MPI_COMM_WORLD);
            clusters.insert(std::string(buf, size));
            if (rank == 0) {
                clusters_it++;
            }
            free(buf);
        }
        for (int ss = 0; ss < sources_size; ss++) {
            int size;
            char* buf;
            if (rank == 0) {
                size = sources_it->length();
                buf = strdup(sources_it->c_str());
            }
            MPI_Bcast(static_cast<void *>( &size), 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(rank!=0){
                buf = static_cast<char*>(malloc(sizeof(char)*size));
            }
            MPI_Bcast(static_cast<void *> (buf), size, MPI_CHAR, 0, MPI_COMM_WORLD);
            sources.insert(std::string(buf,size));;
            if (rank == 0){
                sources_it++;
            }
            free(buf);
        }
        Species nsp = {std::string(species_name, species_name_size), std::string(grade, grade_size)};
        nsp.setClusters(clusters);
        nsp.setSources(sources);
        nsp.setRecordCount(record_count);
        data.insert({std::string(species_name, species_name_size), nsp });
        if (rank == 0)
        {
          it++;
        }
        free(species_name);
        free(grade);
    }
}


void createSubDataset(int rank, Dataset& local_data, Dataset& data, int data_size, int num_procs){
    int start = data_size * rank / num_procs;
    int end = data_size * (rank+1) / num_procs;
    int counter = 0;
    for(auto& pair : data){
        if(counter >= start && counter < end){
            local_data.insert(pair);
            if(counter == end){
                break;
            }
        }
        counter++;
    }
}

void retriveResults(int rank, int num_procs, Dataset& local_data, Dataset& data){
    if(rank == 0){
        for(int irank = 1; irank < num_procs; irank++){
            int size;
            MPI_Recv(static_cast<void *>( &size), 1, MPI_INT, irank, 0,MPI_COMM_WORLD, NULL);
            for(int i = 0; i < size; i++){
                char* species_name;
                int species_name_size;
                char* grade;
                int grade_size;
                MPI_Recv(static_cast<void *>( &species_name_size), 1, MPI_INT, irank, 0, MPI_COMM_WORLD, NULL);
                species_name =static_cast<char*>(malloc(sizeof(char)*species_name_size));
                MPI_Recv(static_cast<void *> (species_name), species_name_size, MPI_CHAR, irank, 0, MPI_COMM_WORLD, NULL);
                MPI_Recv(static_cast<void *>( &grade_size), 1, MPI_INT, irank, 0, MPI_COMM_WORLD, NULL);
                grade = static_cast<char*>(malloc(sizeof(char)*grade_size));
                MPI_Recv(static_cast<void *> (grade), grade_size, MPI_CHAR, irank, 0, MPI_COMM_WORLD, NULL);
                Species& species = data.at(std::string(species_name, species_name_size));
                species.setGrade(std::string(grade, grade_size));
                data[species.getSpeciesName()] = species;
                free(grade);
                free(species_name);
            }
        }
        for(auto& pair: local_data){
            auto species = pair.second;
            data[species.getSpeciesName()] = species;
        }
    }
    else if(rank != 0){
        int size = local_data.size();
        MPI_Send(static_cast<void *>( &size),1,MPI_INT,0,0,MPI_COMM_WORLD);
        for(auto& pair : local_data){
            auto species = pair.second;
            char* species_name;
            int species_name_size;
            char* grade;
            int grade_size;
            species_name = strdup(species.getSpeciesName().c_str());
            species_name_size = species.getSpeciesName().length();
            grade = strdup(species.getGrade().c_str());
            grade_size = species.getGrade().length();
            MPI_Send(static_cast<void *>( &species_name_size), 1, MPI_INT,0, 0, MPI_COMM_WORLD);
            MPI_Send(static_cast<void *>( species_name), species_name_size, MPI_CHAR, 0,0, MPI_COMM_WORLD);
            MPI_Send(static_cast<void *>( &grade_size), 1, MPI_INT, 0,0, MPI_COMM_WORLD);
            MPI_Send(static_cast<void *>( grade), grade_size, MPI_CHAR, 0,0, MPI_COMM_WORLD);
            free(species_name);
            free(grade);
        }
    }
}


int main(int argc, char** argv)
{
    
     
    int ierr;
    int num_procs;
    int rank;
    int threads; 
    Dataset data;
    DistanceMatrix distances;
    GradingParameters params;
    int data_size;
    int distances_size;
    double params_dist;
    int params_size;
    int params_sources;
    Dataset local_data;
    DistanceMatrix local_distances;
    GradingParameters local_params;

    ierr = MPI_Init(&argc, &argv);
    if (ierr != 0) {
        std::cout << "\n";
        std::cout << "ISID - Fatal error!\n";
        std::cout << "  MPI_Init returned ierr = " << ierr << "\n";
        exit(1);
    }
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    
    masterSetup( argc, argv, rank, threads, data, distances, params, data_size, distances_size, params_dist, params_size, params_sources, num_procs);
    broadcastSetup(threads,data_size,distances_size,params_dist,params_size,params_sources);

    local_params = { params_sources, params_size, params_dist };

    broadcastDataset(rank, data_size, data);
    createSubDataset(rank, local_data, data, data_size, num_procs);

    ispecid::IEngine engine(threads, num_procs);
    engine.annotateMPI(local_data, data, distances, params);
    retriveResults(rank, num_procs, local_data, data);

    if(rank == 0){
        for(auto& pair: data){
            PRINT(pair.second.getSpeciesName() << ":" << pair.second.getGrade());
        }

    }
    MPI_Finalize();
    
    return 0;
}