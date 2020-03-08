#ifndef ANNOTATEV6_HPP
#define ANNOTATEV6_HPP

#include <iostream>
#include <limits>
#include <regex>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <miner.hpp>
#include <tsvreader.hpp>
#include <utils.hpp>
using std::string;
using std::vector;


namespace V6{

enum class Grade{
	U, A, B, C, D, E1, E2
};

template<typename K, typename V> 
using umap = std::unordered_map<K,V>;

typedef struct bdata{
	double distance;
	string neighbour;
}BINData;



class Entry{
	public:
	/*
		Desc: 
			- data struture to store the Speciesrmation of one record and respective grade
		Comm: 
			- unnecessary data storage, from each entry we only need (species_name or id), 
			  bin_name and institution
	*/
	csv::CSVRow cols;
	Grade grade = Grade::D; 
	Entry(csv::CSVRow _cols, Grade _grade = Grade::D):
		cols(_cols), grade(_grade){};
	
	std::string_view operator[](size_t n) const{
		return cols.get_string_view(n);
	}
};

class Species{
	public:
	/*
		Desc: 
			- data struture to store the Speciesrmation necessary to the algorithm itself.
		Comm: 
			- substitution of species_name for an int would reduce the number of string comparison.
	*/
	std::string_view species_name;
	std::vector<Entry> specimens;
	std::unordered_set<std::string_view> bins;
	std::unordered_set<std::string_view> institution;
	Grade grade;
	Species():grade(Grade::U){}
	Species(
		std::string_view _species_name, 
		std::unordered_set<std::string_view> _bins = {}, 
		std::unordered_set<std::string_view> _institution = {},
		Grade _grade = Grade::U):
		species_name(_species_name), bins(_bins), institution(_institution), grade(_grade){};

	void push_back(Entry specimen){
		specimens.push_back(specimen);
		bins.insert( specimen.cols["bin_uri"].get<csv::string_view>());
		institution.insert( specimen.cols["institution_storing"].get<csv::string_view>());
		species_name = specimen.cols["species_name"].get<csv::string_view>();
	}
	friend std::ostream & operator << (std::ostream &strm, const Species &obj) {
		strm << "Species name: " << std::string(obj.species_name) << std::endl <<
			"Grade: " << static_cast<typename std::underlying_type<Grade>::type>(obj.grade) << std::endl<<
			"Bins: {" << std::endl;
		for(auto b : obj.bins)
			strm << "\t" << b << std::endl;
		strm << "}" << std::endl;
		return strm;
	}
};




template <typename T>
using matrix = std::vector<std::vector<T>>;
void readTSV(std::string file_path, vector<string>& header , std::vector<Entry> &data);
bool speciesPerBIN(umap<std::string_view, Species>, string& BINname,int species_ind, int bin_ind);
Grade speciesCongruence(std::vector<Entry>& data,int institution_ind);
std::vector<Entry> remove_empty_col(std::vector<Entry> data, int index);
void _allInConnComp(std::vector<std::vector<float>> mat, int next, int bin, vector<int>& visited);
bool checkAllInConnComp(std::vector<std::vector<float>> mat);
BINData parseBoldBINdata(string bin);
Grade BINnearestNeighb(std::unordered_set<std::string_view> allBINsOfThisSpecies, umap<std::string_view,Species> data, int bin_ind, int species_ind);
void annotate(string& file_path);


}
#endif
