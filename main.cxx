#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <benchmark/benchmark.h>
#include <tuple>
#include <curl/curl.h>
#include <csv.hpp>
#include "include/tsvreader/TSVReader.hpp"
using std::string;
using std::vector;

#define PRINT(X) std::cout << X << std::endl;

enum class Grade{
	U, A, B, C, D, E1, E2
};

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
	Grade grade = Grade::U;
	row(csv::CSVRow r, Grade g = Grade::U) : data(r) {};
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
	std::vector<std::string> bins;
	info(std::string_view species_name_, Grade grade_, std::vector<std::string> bins_):
		species_name(species_name_), grade(grade_), bins(bins_){};
};

typedef vector<info> tinfo;
typedef vector<row> rows;



typedef struct memStruct {
  char *memory;
  size_t size;
}MemoryStruct;
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = ( MemoryStruct *)userp;
 
  char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
 
MemoryStruct getPage(const char* url)
{
  CURL *curl_handle;
  CURLcode res;
 
  MemoryStruct chunk;
 
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* init the curl session */ 
  curl_handle = curl_easy_init();
 
  /* specify URL to get */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, url); 
 
  /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
  /* get it! */ 
  res = curl_easy_perform(curl_handle);
 
  /* check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */ 
 
    printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
  }
 
  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle);
 
  /* we're done with libcurl, so clean it up */ 
  curl_global_cleanup();
 
 return chunk;
}




template <class T>
T argParse(int argc, char *argv[], std::string flag, T defaultValue)
{
	for (int i = 0; i < argc; ++i){
		PRINT(argv[i])
		if (!flag.compare(argv[i]))
		{
			std::istringstream ss(argv[i + 1]);

			T value;
			ss >> value;

			return value;
		}
	}
	return defaultValue;
}



void readTSV(std::string file_path, vector<string>& header , rows &data)
{

	TSVReader reader(file_path);
	header = reader.get_col_names();
	for (auto& entry : reader)
	{
		row r(entry);
		data.push_back(entry);
	}
}

template <class _ForwardIterator, class _Tp>
int findi(_ForwardIterator __first, _ForwardIterator __last, const _Tp &__value)
{
	auto elem = std::find(__first, __last, __value);
	auto dist = std::distance(__first, elem);
	auto size = std::distance(__first, __last);
	return dist < size ? dist : -1;
}




/*
	Current complexity: O(N)
*/
template <class _Tp>
std::vector<_Tp> unique_elems(rows data, int index){
	vector<_Tp> unique_vec;
	std::unordered_set<_Tp> unique_set;
	for(auto & row : data){
		unique_set.insert(row.data.get_string_view(index));
	}
	unique_vec.assign(unique_set.begin(), unique_set.end());
	return unique_vec;
}


/*
	Current complexity: O(N)
*/
void enoughData(tinfo& out, rows data, int species_ind, int bin_ind, int index){
	info& current = out.at(index);
	auto count = std::count_if(
		std::begin(data), 
		std::end(data), 
		[species_ind, current](row elem){
			return elem.data.get_string_view(species_ind).compare(current.species_name) == 0;
		});
	if( count <= 3){
		current.grade = Grade::D;
	}
	
}



/*
	Current complexity: O(N + UNIQUE)
	The set <insert> returns <bool> we can push in to the vector 
	if true eliminating the UNIQUE component from the complexity
*/
void allSeqs1BIN(rows data, int species_ind, info& i, int bin_ind){
	std::unordered_set<std::string_view> unique_set;
	std::string_view& sp = i.species_name;
	for(auto & entry : data){
		if(entry.data.get_string_view(species_ind) == sp){
			unique_set.insert(entry.data.get_string_view(bin_ind));
		}
	}
	i.bins.assign(unique_set.begin(), unique_set.end());
}

/*
	Current complexity: O(N)
	We can return when the size of the set is bigger than 1 but 
	it requires one extra comparison each iteration
*/
bool speciesPerBIN(rows data, string& BINname,int species_ind, int bin_ind)
{
	int count;
	std::unordered_set<std::string_view> unique_set;
	for(auto & entry : data){
		if(entry.data.get_string_view(bin_ind) == BINname){
			unique_set.insert(entry.data.get_string_view(species_ind));
		}
	}
	return unique_set.size() == 1;
}


/*
	Current complexity: O(N + #mod_rows)
*/
void speciesCongruence(rows& data, std::string_view sp, int species_ind, int institution_ind){
	std::unordered_set<std::string_view> unique_set;
	vector<int> mod_rows;
	int count = data.size();
	for (size_t i = 0; i < count; i++) {
		if(sp == data[i].data.get_string_view(species_ind) && data[i].data.get_string_view(institution_ind) != "NAN") {
			unique_set.insert(data[i].data.get_string_view(institution_ind));
			mod_rows.push_back(i);
		}
	}
	if (unique_set.size()==1 || mod_rows.size() == 1) {
		for(auto item : mod_rows) {
			data[item].grade = Grade::B;
		}
	}
    else{
		for(auto item : mod_rows) {
			data[item].grade = Grade::A;
		}
	}
}

void remove_empty_col(rows& data, int index){
	data.erase(
		std::remove_if(	std::begin(data), std::end(data),
			[index](row x) {
				return x.data[index] == "";
			}),
		std::end(data));
}




void _allInConnComp(std::vector<std::vector<float>> mat, int next, int bin, vector<int>& visited){
	visited[next] = bin;
	std::vector<float> edges = mat[next];
	for (size_t i = 0; i < mat.size(); i++)
	{
		if (visited[i] == 0 && edges[i] != 0)
		{
			_allInConnComp(mat, i, bin, visited);
		}
	}
}



/*
	Use Boost instead...
*/
bool checkAllInConnComp(std::vector<std::vector<float>> mat){
	vector<int> visited; 
	int bin = 1;
	for(int v = 0; v < mat.size(); v++) 
        visited.push_back(0);
	for(int i = 0; i < mat.size(); i++){
		if(visited[i] == 0)
		{
			_allInConnComp(mat, i, bin, visited);
			bin++;
		}
	}
	return std::all_of(std::begin(visited), std::end(visited), [](int v){return v == 1;});
}


typedef struct bdata{
	double distance;
	string neighbour;
}BINData;
BINData parseBoldBINdata(string bin){
	string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + bin);
	MemoryStruct ms = getPage(url.c_str());
	string s(ms.memory);
	free(ms.memory);
	std::smatch matches;
	std::regex dist("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%");
	std::regex nearest("Nearest BIN URI:</th>\\s*<td>(.*)</td>");
	std::regex_search (s,matches,nearest);
	string nbin = matches[1];
	std::regex_search (s,matches,dist);
	float d = std::stof(matches[1]);
	BINData bd;
	bd.distance = d;
	bd.neighbour = nbin;
	return bd;
}

template <typename T>
using matrix = std::vector<std::vector<T>>;

Grade BINnearestNeighb(vector<string> allBINsOfThisSpecies, rows data, int bin_ind, int species_ind)
{
	int size = allBINsOfThisSpecies.size();
	Grade grade=Grade::E2;
	std::unordered_set<std::string_view> indexes;
	matrix<float> outMatrix(size, vector<float>(size,0.0));
	for(int i = 0; i < data.size(); i++) {
		/*
			Fetch data into local variable
			It is not necessary to iterate 
			over all data if size if bigger than 1 return...
		*/
		for(auto& bin: allBINsOfThisSpecies) {
			if (data[i].data.get_string_view(bin_ind) == bin) {
				indexes.insert(data[i].data.get_string_view(species_ind));
			}
		}
	}
	if (indexes.size() == 1) {
		for(int i=0; i<allBINsOfThisSpecies.size(); i++){
			auto BINdata = parseBoldBINdata(allBINsOfThisSpecies[i]);
			auto ind = findi(std::begin(allBINsOfThisSpecies), std::end(allBINsOfThisSpecies), BINdata.neighbour);
			if( ind != -1 && BINdata.distance <=2) {
				outMatrix[i][ind]=BINdata.distance;
				outMatrix[ind][i]=BINdata.distance;
			}
		}
		if( checkAllInConnComp(outMatrix) ) {
			grade=Grade::C;
		}
	}
	return grade;
}

void fill(tinfo& out, vector<std::string_view> species_names){
	std::transform( std::begin(species_names), std::end(species_names), std::back_inserter(out),
		[](std::string_view elem){
			info t(elem, Grade::U, {});
			return t;
		}
	);
}

void copy_if_D(tinfo out, tinfo& grade_D_species){
	std::copy_if ( out.begin(), out.end(), std::back_inserter(grade_D_species), 
			[](info i){
				return i.grade == Grade::D;
			} 
		);
}

void annotate(string& file_path){
	rows data;
	vector<string> header;
	readTSV(file_path, header, data);
	/*
		Current complexity: O(N*(#str1 + #str2 + #str3))
		Can be done with one travesal
		Expected complexity: O(N*(MAX({str1,str2,str3})))	
	*/
	int bin_ind         = findi(std::begin(header), std::end(header), "bin_uri");
	int species_ind     = findi(std::begin(header), std::end(header), "species_name");
	int institution_ind = findi(std::begin(header), std::end(header), "institution_storing");
	remove_empty_col(data, species_ind);	
	/*
		Current complexity: O(2*N)
		Can be done with one travesal
		Expected complexity: O(N)
	*/
	auto species_names  = unique_elems<std::string_view>(data, species_ind);
	auto bin_names = unique_elems<std::string_view>(data, bin_ind);
	tinfo out;
	fill(out,species_names);

	
	for (size_t i = 0; i < out.size(); i++) {
		enoughData(out, data, species_ind, bin_ind, i);
		tinfo grade_D_species;
		/*
			Unnecessary, we visit several times the same species
			1) enoughData should return <bool> and if it is true proceed with annotation.
			2) enoughData should return <string> of the species and after checking all species
			   proceed with annotation.
			3) it necessary to even annotate as D? The default grade can be D and all species 
			   that aren't will be annotated with A,B,C,E1 or E2. It is only necessary to know 
			   that a species name is graded as D to the next step
		*/
		copy_if_D(out, grade_D_species);
		
		for (auto& d_species : grade_D_species) {
			for ( auto& entry: data) {
				if(entry.data[species_ind] == d_species.species_name){
					entry.grade = Grade::D;
				}
			}
		}
	}

	
	for (size_t i = 0; i < out.size(); i++) {
		info& current = out[i];
		/* It is possible to eliminate this verification */
		if (current.grade != Grade::D) {
			allSeqs1BIN(data,species_ind,current,bin_ind);
			if(current.bins.size() == 1){
				bool BINSpeciesConcordance = speciesPerBIN(data,current.bins[0],species_ind,bin_ind);
				if(BINSpeciesConcordance){
					speciesCongruence(data,current.species_name,species_ind,institution_ind);
				}
				else{
					for(auto & item : data) {
						if (item.data.get_string_view(species_ind) == current.species_name)
							item.grade = Grade::E1;
					}
				}
			}
			else {
				Grade g = BINnearestNeighb(current.bins, data, bin_ind, species_ind);
				for(auto& item: data) {
					if( item.data.get_string_view(species_ind) == current.species_name)
						item.grade = g;
				}
			}
		}
	}



	PRINT("DATA")
	for (size_t i = 0; i < data.size(); i++)
	{
		PRINT(data[i].data.get_string_view(1) << ";" << static_cast<typename std::underlying_type<Grade>::type>(data[i].grade));
	}
}

int main(int argc, char **argv)
{
	std::string file_path = argParse<std::string>(argc, argv, "--data", "Canidae_BIN.tsv");
	
}

