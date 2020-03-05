#ifndef ANNOTATEV2_HPP
#define ANNOTATEV2_HPP

#include <iostream>
#include <limits>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <numeric>
#include <miner.hpp>
#include <tsvreader.hpp>
#include <utils.hpp>
using std::string;
using std::vector;


namespace V2{

enum class Grade{
	U, A, B, C, D, E1, E2
};

typedef struct bdata{
	double distance;
	string neighbour;
}BINData;

class row{
	public:
	/*
		Desc: 
			- data struture to store the information of one record and respective grade
		Comm: 
			- unnecessary data storage, from each entry we only need (species_name or id), 
			  bin_name and institution
	*/
	csv::CSVRow data;
	Grade grade = Grade::D; 
	row(csv::CSVRow r, Grade g = Grade::D) : data(r) {};
};

class info{
	public:
	/*
		Desc: 
			- data struture to store the information necessary to the algorithm itself.
		Comm: 
			- substitution of species_name for an int would reduce the number of string comparison.
	*/
	std::string_view species_name;
	Grade grade;
	std::vector<std::string_view> bins;
	info(std::string_view species_name_, Grade grade_, std::vector<std::string_view> bins_):
		species_name(species_name_), grade(grade_), bins(bins_){};
};


template <typename T>
using matrix = std::vector<std::vector<T>>;
typedef vector<info> tinfo;
typedef vector<row> rows;



void readTSV(std::string file_path, vector<string>& header , rows &data);
template <class _ForwardIterator, class _Tp>
int findi(_ForwardIterator __first, _ForwardIterator __last, const _Tp &__value);

template <class _Tp>
std::vector<_Tp> unique_elems(rows data, int index);

void enoughData(info& out, rows data, int species_ind);
void allSeqs1BIN(rows data, int species_ind, info& i, int bin_ind);
bool speciesPerBIN(rows data, string& BINname,int species_ind, int bin_ind);
void speciesCongruence(rows& data, std::string_view sp, int species_ind, int institution_ind);
void remove_empty_col(rows& data, int index);
void _allInConnComp(std::vector<std::vector<float>> mat, int next, int bin, vector<int>& visited);
bool checkAllInConnComp(std::vector<std::vector<float>> mat);
BINData parseBoldBINdata(string bin);
Grade BINnearestNeighb(vector<string> allBINsOfThisSpecies, rows data, int bin_ind, int species_ind);
void fill(tinfo& out, vector<std::string_view> species_names);
void copy_if_D(tinfo out, tinfo& grade_D_species);
void annotate(string& file_path);


}
#endif
