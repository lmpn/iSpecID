#ifndef ANNOTATEV3_HPP
#define ANNOTATEV3_HPP

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


namespace V3{

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
	std::vector<std::string_view> bins;
	Grade grade;
	Species(std::string_view _species_name, std::vector<std::string_view> _bins = {}, Grade _grade = Grade::U):
		species_name(_species_name), bins(_bins), grade(_grade){};
};


template <typename T>
using matrix = std::vector<std::vector<T>>;



void readTSV(std::string file_path, vector<string>& header , std::vector<Entry> &data);
template <class _ForwardIterator, class _Tp>
int findi(_ForwardIterator __first, _ForwardIterator __last, const _Tp &__value);

template <class _Tp>
std::vector<_Tp> unique_elems(std::vector<Entry> data, int index);

bool enoughData(Species& out, std::vector<Entry> data, int species_ind);
bool allSeqs1BIN(std::vector<Entry> data, int species_ind, Species& i, int bin_ind);
bool speciesPerBIN(std::vector<Entry> data, string& BINname,int species_ind, int bin_ind);
void speciesCongruence(std::vector<Entry>& data, std::string_view sp, int species_ind, int institution_ind);
std::vector<Entry> remove_empty_col(std::vector<Entry> data, int index);
void _allInConnComp(std::vector<std::vector<float>> mat, int next, int bin, vector<int>& visited);
bool checkAllInConnComp(std::vector<std::vector<float>> mat);
BINData parseBoldBINdata(string bin);
Grade BINnearestNeighb(vector<string> allBINsOfThisSpecies, std::vector<Entry> data, int bin_ind, int species_ind);
void fill(std::vector<Species>& out, vector<std::string_view> species_names);
void copy_if_D(std::vector<Species> out, std::vector<Species>& grade_D_species);
void annotate(string& file_path);


}
#endif
