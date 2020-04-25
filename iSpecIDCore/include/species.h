#ifndef SPECIES_H
#define SPECIES_H
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "record.h"

class Species{
    public:
    std::string species_name;
    std::vector<Record> specimens;
    std::unordered_map<std::string, int> bins;
    std::unordered_set<std::string> institution;
    std::string grade;

    Species():grade("U"){}

    Species(
        std::string species_name,
        std::unordered_map<std::string, int> _bins,
        std::unordered_set<std::string> _institution,
        std::string _grade = "U") :
        species_name(species_name), bins(_bins), institution(_institution), grade(_grade){};

    void push_back(Record specimen){
        specimens.push_back(specimen);
        bins[specimen["bin_uri"]] += 1;
        institution.insert( specimen["institution_storing"]);
        species_name = specimen["species_name"];
        grade = specimen["grade"];
    }

    friend std::ostream & operator << (std::ostream &strm, const Species &obj) {
        strm << "Species name: " << std::string(obj.species_name) << std::endl <<
            "utils::Grade: " << obj.grade << std::endl<<
            "Bins: {" << std::endl;
        for(auto b : obj.bins)
            strm << "\t" << b.first << " count: " << b.second << std::endl;
        strm << "}" << std::endl;
        return strm;
    }
};
#endif
