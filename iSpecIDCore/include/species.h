#ifndef SPECIES_H
#define SPECIES_H
#include <string>
#include <vector>
#include <unordered_set>
#include <record.h>
#include <utils.h>

class Species{
    public:
    std::string species_name;
    std::vector<Record> specimens;
    std::unordered_set<std::string> bins;
    std::unordered_set<std::string> institution;
    std::string grade;

    Species():grade("U"){}

    Species(
        std::string_view _species_name,
        std::unordered_set<std::string> _bins = {},
        std::unordered_set<std::string> _institution = {},
        std::string _grade = "U") :
        species_name(_species_name), bins(_bins), institution(_institution), grade(_grade){};

    void push_back(Record specimen){
        specimens.push_back(specimen);
        auto bin = specimen["bin_uri"];
        bins.insert(bin);
        institution.insert( specimen["institution_storing"]);
        species_name = specimen["species_name"];
    }

    friend std::ostream & operator << (std::ostream &strm, const Species &obj) {
        strm << "Species name: " << std::string(obj.species_name) << std::endl <<
            "utils::Grade: " << obj.grade << std::endl<<
            "Bins: {" << std::endl;
        for(auto b : obj.bins)
            strm << "\t" << b << std::endl;
        strm << "}" << std::endl;
        return strm;
    }
};
#endif
