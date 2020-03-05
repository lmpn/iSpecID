#include <annotateV4.hpp>
#include <functional>

namespace V4{
using namespace std::placeholders;


void readTSV(std::string file_path, std::vector<string>& header , std::vector<Entry> &data)
{

	TSVReader reader(file_path);
	header = reader.get_col_names();
	for (auto& row : reader)
	{
		Entry e(row);
		data.push_back(e);
	}
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("Number of entries read: " << data.size());
		PRINT("*--------------------------------------*");
	#endif
}

template<class K, class V, class FN>
umap<K,std::vector<V>> 
keyBy(std::vector<V> data, FN getKey){
	umap<K,std::vector<V>> result;
	size_t count = data.size();
	for (size_t i = 0; i < count; i++)
	{
		V& item = data[i];
		K key = getKey(data[i]);
		std::vector<V>& current = result[key];
		current.push_back(item);
	}
	return result;
}




std::unordered_map<std::string,size_t> 
create_indixes(std::vector<string> header){
	std::unordered_map<std::string,size_t> indexes;
	size_t count = 0;
	for (auto& elem : header)
	{
		indexes.insert({elem, count++});
	}
	return indexes;
}


/*
	Current complexity: O(N)
*/
template <class _Tp>
std::vector<_Tp> unique_elems(vector<Entry> data, int index){
	vector<_Tp> unique_vec;
	std::unordered_set<_Tp> unique_set;
	for(auto & entry : data){
		auto ires = unique_set.insert(entry[index]);
		if(ires.second)
			unique_vec.push_back(entry[index]);
	}

	return unique_vec;
}


/*
	Current complexity: O(N)
	We can return when the size of the set is bigger than 1 but 
	it requires one extra comparison each iteration
*/
bool speciesPerBIN(vector<Entry> data, std::string_view BINname, int species_ind, int bin_ind)
{
	std::unordered_set<std::string_view> unique_set;
	for(auto & entry : data){
		if(entry[bin_ind] == BINname){
			unique_set.insert(entry[species_ind]);
			if(unique_set.size()>1) return false;
		}
	}
	return true;
}


/*
	Current complexity: O(N + #mod_rows)
*/
Grade speciesCongruence(vector<Entry>& data, int institution_ind){
	std::unordered_set<std::string_view> unique_set;
	vector<Entry> mod_rows;
	int count = data.size();
	for (auto& item : data){
		unique_set.insert(item[institution_ind]);
	}
	if (unique_set.size()==1) {
		return Grade::B;
	}
    else{
		return Grade::A;
	}
}

vector<Entry> remove_empty_col(vector<Entry> data, int index){
	vector<Entry> result;
	std::remove_copy_if(std::begin(data), std::end(data), std::back_inserter(result),
			[index](Entry item) {
				return item[index].empty();
			});
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("Number of non-removed entries: " << result.size());
		PRINT("*--------------------------------------*");
	#endif
	return result;
}





void _allInConnComp(matrix<float> mat, int next, int bin, std::vector<int>& visited){
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
bool checkAllInConnComp(matrix<float> mat){
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

template <class _ForwardIterator, class _Tp>
int findi(_ForwardIterator __first, _ForwardIterator __last, const _Tp &__value)
{
	auto elem = std::find(__first, __last, __value);
	auto dist = std::distance(__first, elem);
	auto size = std::distance(__first, __last);
	return dist < size ? dist : -1;
}

BINData parseBoldBINdata(std::string_view bin){
	BINData bd;
	bd.distance = std::numeric_limits<int>::max();
	bd.neighbour = "";
	miner mn;
	string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
	try{
		string s = mn.getPage(url.c_str());
		std::smatch matches;
		std::regex dist("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%");
		std::regex nearest("Nearest BIN URI:</th>\\s*<td>(.*)</td>");
		std::regex_search (s,matches,nearest);
		string nbin = matches[1];
		std::regex_search (s,matches,dist);
		float d = std::stof(matches[1]);
		bd.distance = d;
		bd.neighbour = nbin;
	}catch (const std::exception& e) { 
		PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
	}
	return bd;
}


Grade BINnearestNeighb(std::vector<std::string_view> allBINsOfThisSpecies, std::vector<Entry> data, int bin_ind, int species_ind)
{
	Grade grade=Grade::E2;
	std::unordered_set<std::string_view> species;
	size_t count = data.size();
	for(size_t i = 0; i < count; i++) {
		/*
			Fetch data into local variable
			It is not necessary to iterate 
			over all data if size if bigger than 1 return...
		*/
		std::string_view dataBin = data[i][bin_ind];
		for(auto& bin: allBINsOfThisSpecies) {
			if (dataBin == bin) {
				species.insert(data[i][species_ind]);
				if(species.size() > 1 ){
					return grade;
				}
				break;
			}
		}
	}
	count = allBINsOfThisSpecies.size();
	matrix<float> outMatrix(count, std::vector<float>(count,0.0));
	for(size_t  i= 0; i < count; i++){
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
	return grade;
}



void annotate(string& file_path){
	std::vector<Entry> data;
	std::vector<std::string> header;
	readTSV(file_path, header, data);
	umap<std::string,size_t> indexes = create_indixes(header);
	size_t bin_ind         = indexes["bin_uri"];
	size_t species_ind     = indexes["species_name"];
	size_t institution_ind = indexes["institution_storing"];
	data = remove_empty_col(data, species_ind);	
	auto getKey = [](Entry item, size_t index) { return item[index];};
	auto getKeySpecies = std::bind(getKey, _1, species_ind);
	auto md = keyBy<std::string_view, Entry>(data, getKeySpecies);


	for(auto& pair : md){
		auto name = pair.first;
		auto& specimens = pair.second;
		if(specimens.size() > 3){
			auto getKeyBin = std::bind(getKey, _1, bin_ind);
			auto bins = unique_elems<std::string_view>(specimens, bin_ind);
			Grade grade = Grade::E1;
			if(bins.size() == 1){
				bool BINSpeciesConcordance = speciesPerBIN(data,*std::begin(bins),species_ind,bin_ind);
				if(BINSpeciesConcordance){
					grade = speciesCongruence(specimens,institution_ind);
				}
			}else{
				grade = BINnearestNeighb(bins, data, bin_ind, species_ind);	
			}
			for(auto & item: specimens) {
				item.grade = grade;
			}
		}
	}

	#ifdef DEBUG
	PRINT("DATA")
	for (auto& item : md)
	{
		for(auto& sitem: item.second)
			PRINT(sitem[1] << ";" << static_cast<typename std::underlying_type<Grade>::type>(sitem.grade));
	}
	#endif
}

}