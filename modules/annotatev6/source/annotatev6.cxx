#include <annotateV6.hpp>
#include <functional>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

namespace V6{
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


template< class I>
I id(I arg){
	return arg;
}


template<class K, class I, class O, class GET>
umap<K,O> 
keyBy(I data, GET getKey){
	umap<K,O> result;
	static_assert(std::is_member_function_pointer<decltype(&O::push_back)>::value,
                  "No push_back."); 
	//size_t count = data.size();
	for (auto& i : data)
	{
		K key = getKey(i);
		O& current = result[key];
		current.push_back(i);
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



/*
	Current complexity: O(N + #mod_rows)
*/
Grade speciesCongruence(vector<Entry>& data, int institution_ind){
	std::unordered_set<std::string_view> unique_set;
	vector<Entry> mod_rows;
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


bool speciesPerBIN(umap<std::string_view, Species> data, std::string_view BINname, int species_ind, int bin_ind)
{
	std::unordered_set<std::string_view> unique_set;
	size_t count = 0;
	for(auto & entry : data){
		auto fst = entry.second.bins.begin();
		auto lst = entry.second.bins.end();
		auto result = std::find( fst, lst, BINname);
		if(lst != result && (++count) == 2){
			return false;
		}
	}
	return true;
}

template<class T>
bool has_intersection(std::unordered_set<T> o1, std::unordered_set<T> o2){
	size_t o1s = o1.size();
	size_t o2s = o2.size();
	if(o1s <= o2s){
		auto o2e = o2.end();
		for (auto i = o1.begin(); i != o1.end(); i++) {
			if (o2.find(*i) != o2e) return true;
		}
	}else{
		auto o1e = o1.end();
		for (auto i = o2.begin(); i != o2.end(); i++) {
			if (o1.find(*i) != o1e) return true;
		}
	}
	return false;
}

Grade BINnearestNeighb(std::unordered_set<std::string_view> allBINsOfThisSpecies, umap<std::string_view,Species> data, int bin_ind, int species_ind)
{
	Grade grade=Grade::E2;
	size_t count = data.size();
	size_t ctr = 0;
	for(auto& pair : data){
		auto bins = pair.second.bins;
		if(has_intersection(bins,allBINsOfThisSpecies)){
			ctr+=1;
			if(ctr>1) return grade;
		}
	}
	/*
	for(size_t i = 0; i < count; i++) {
		/*
			Fetch data into local variable
			It is not necessary to iterate 
			over all data if size if bigger than 1 return...
		
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
	}*/
	count = allBINsOfThisSpecies.size();
	auto it = allBINsOfThisSpecies.begin();
	//matrix<float> outMatrix(count, std::vector<float>(count,0.0));
	typedef boost::adjacency_list< boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, double>> ugraph;
	ugraph graph(count);
	for(size_t  i= 0; i < count; i++){
		auto BINdata = parseBoldBINdata(*it);
		auto ind = findi(std::begin(allBINsOfThisSpecies), std::end(allBINsOfThisSpecies), BINdata.neighbour);
		if( ind != -1 && BINdata.distance <=2) {
			boost::add_edge(i, ind, BINdata.distance, graph);
			boost::add_edge(ind, i, BINdata.distance, graph);
		}
		it++;
	}
	std::vector<int> component (count);
	size_t num_components = boost::connected_components (graph, &component[0]);
	if( num_components == 1){
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
	data = remove_empty_col(data, species_ind);	
	auto getKey = [](Entry item, size_t index) { return item[index];};
	auto getKeySpecies = std::bind(getKey, _1, species_ind);
	auto md = keyBy<std::string_view,vector<Entry>,Species>(data, getKeySpecies);


	for(auto& pair : md){
		auto& species = pair.second;
		Grade grade = Grade::D;
		if(species.specimens.size() > 3){
			Grade grade = Grade::E1;
			if(species.bins.size() == 1){
				bool BINSpeciesConcordance = speciesPerBIN(md,*species.bins.begin(),species_ind,bin_ind);
				if(BINSpeciesConcordance){
					grade = species.institution.size()==1 ? Grade::B : Grade::A;
					//speciesCongruence(species.specimens,institution_ind);
				}
			}else{
				grade = BINnearestNeighb(species.bins, md, bin_ind, species_ind);	
			}
			species.grade = grade;
		}else{
			species.grade = grade;
		}
	}

	#ifdef DEBUG
	PRINT("DATA")
	for (auto& item : md)
	{
		for(auto& sitem: item.second.specimens)
			PRINT(sitem[1] << ";" << static_cast<typename std::underlying_type<Grade>::type>(item.second.grade));
	}
	#endif
}

}