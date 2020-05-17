#ifndef DATATYPES_H
#define DATATYPES_H 
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

namespace ispecid{ namespace datatypes{
    
    
    typedef struct grading_params{
        int min_sources = 2;
        int min_size = 3;
        double max_distance = 2.0; 
    }GradingParameters;


    typedef boost::adjacency_list<
        boost::listS,
        boost::vecS,
        boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double>
    > ugraph;

    typedef struct neighbour{
        std::string clusterA;
        std::string clusterB;
        double distance;
    }Neighbour;

    class Record{
        private:
        std::string species_name;
        std::string cluster;
        std::string source;
        std::string grade;

        public:
        Record(std::string species_name, std::string cluster, std::string source, std::string grade):
            species_name(species_name), cluster(cluster), source(source), grade(grade)
        {
        };

        static std::string getSpeciesName(Record& record){
            return record.getSpeciesName();
        }

        std::string getSpeciesName(){
            return species_name;
        }
        std::string getCluster(){
            return cluster;
        }
        std::string getSource(){
            return source;
        }
        std::string getGrade(){
            return grade;
        }
        static bool goodRecord(Record& record){
            return record.species_name.empty() || record.cluster.empty() || record.source.empty();
        }
        std::string toString(){
            return species_name + ";" + cluster+ ";" +source+ ";" + grade;
        }
    };

    class Species{
        private:
        std::string species_name;
        std::unordered_set<std::string> clusters;
        std::unordered_set<std::string> sources;
        std::string grade;
        int record_count;

        public:
        Species(std::string species_name, std::string grade):
            species_name(species_name), grade(grade), record_count(0)
        {
        };


        void setGrade( std::string grade){
            this->grade = grade;
        };

        std::string getFirstCluster(){
            return *clusters.begin();
        }
        std::string getSpeciesName(){
            return species_name;
        }
        const std::unordered_set<std::string>& getClusters(){
            return clusters;
        }
        const std::unordered_set<std::string>&  getSources(){
            return sources;
        }
        std::string getGrade(){
            return grade;
        }
        int recordCount(){
            return record_count;
        }
        int sourcesCount(){
            return sources.size();
        }
        int clustersCount(){
            return clusters.size();
        }

        static Species fromRecord(Record& record){
            Species species(record.getSpeciesName(), record.getGrade());
            addRecord(species,record);
            return species;
        }

        static void addRecord(Species& species, Record& record){
            species.record_count++;
            species.sources.insert(record.getSource());
            species.clusters.insert(record.getCluster());
        }
         std::string toString(){
            std::string out = species_name; 
            out = out + " sources:{";
            for(auto& s :sources){
                out= out + s + " "; 
            }
            out = out + "}";
            out = out + " clusters:{";
            for(auto& s :clusters){
                out= out + s + " "; 
            }
            out = out + "} ";
            out = out + "grade: " + grade;
            out = out + "size: " + std::to_string(record_count);
            return out;
        }
    };

using Dataset = std::unordered_map<std::string, Species>;
using DistanceMatrix = std::unordered_map<std::string, Neighbour>;
}}
#endif