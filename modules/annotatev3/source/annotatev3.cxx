#include <annotateV3.hpp>
namespace V3{
void readTSV(std::string file_path, vector<string>& header , std::vector<Entry> &data) {

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
*/
bool enoughData(Species& current, vector<Entry> data, int species_ind){
	size_t count = 0;
	std::string_view sp = current.species_name;
	for(auto& elem : data)
		if(elem[species_ind] == sp){
			count++;
			if(count > 3){
				return true;
			}
		}
	current.grade = Grade::D;
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("Species "<< current.species_name << " marked with D");
		PRINT("*--------------------------------------*");
	#endif
	return false;
}



/*
	Current complexity: O(N + UNIQUE)
	The set <insert> returns <bool> we can push in to the vector 
	if true eliminating the UNIQUE component from the complexity
*/
bool allSeqs1BIN(vector<Entry> data, int species_ind, Species& i, int bin_ind){
	std::unordered_set<std::string_view> unique_set;
	std::string_view& sp = i.species_name;
	std::vector<std::string_view>& bins = i.bins;
	for(auto & item : data){
		if(item[species_ind] == sp){
			auto bin = item[bin_ind];
			auto ipair = unique_set.insert(bin);
			if(ipair.second){
				bins.push_back(bin);
			}
		}
	}
	bool result = bins.size() == 1;
	return result;
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
void speciesCongruence(vector<Entry>& data, std::string_view sp, int species_ind, int institution_ind){
	std::unordered_set<std::string_view> unique_set;
	vector<int> mod_rows;
	int count = data.size();
	for (size_t i = 0; i < count; i++) {
		if(sp == data[i][species_ind] && data[i][institution_ind] != "NAN") {
			unique_set.insert(data[i][institution_ind]);
			mod_rows.push_back(i);
		}
	}
	if (unique_set.size()==1 || mod_rows.size() == 1) {
		for(auto item : mod_rows) {
			data[item].grade = Grade::B;
			#ifdef DEBUG
				PRINT("*--------------------------------------*");
				PRINT("Species "<< sp << " marked as B");
				PRINT("*--------------------------------------*");
			#endif
		}
	}
    else{
		for(auto item : mod_rows) {
			data[item].grade = Grade::A;
			#ifdef DEBUG
				PRINT("*--------------------------------------*");
				PRINT("Species "<< sp << " marked as A");
				PRINT("*--------------------------------------*");
			#endif
		}
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





void _allInConnComp(matrix<float> mat, int next, int bin, vector<int>& visited){
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
	int count = 0;
	while(count<3){
		try{
			string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
			string s = mn.getPage(url.c_str());
			std::smatch matches;
			std::regex dist("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%");
			std::regex nearest("Nearest BIN URI:</th>\\s*<td>(.*)</td>");
			std::regex_search (s,matches,nearest);
			string nbin = matches[1];
			PRINT("bin "<<nbin);
			std::regex_search (s,matches,dist);
			PRINT("d " << matches[1]);
			float d = std::stof(matches[1]);
			bd.distance = d;
			bd.neighbour = nbin;
		}catch (const std::exception& e) { 
			std::cout << e.what(); 
			count++;
		}

	}
	return bd;
}


Grade BINnearestNeighb(std::vector<std::string_view> allBINsOfThisSpecies, vector<Entry> data, int bin_ind, int species_ind)
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
	matrix<float> outMatrix(count, vector<float>(count,0.0));
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

void fill(std::vector<Species>& out, vector<std::string_view> species_names){
	std::transform( std::begin(species_names), std::end(species_names), std::back_inserter(out),
		[](std::string_view elem){
			Species sp(elem);
			return sp;
		}
	);
}




void annotate(string& file_path){
	std::vector<Entry> data;
	std::vector<std::string> header;
	readTSV(file_path, header, data);
	umap<std::string,size_t> indexes = create_indixes(header);
	size_t bin_ind         = indexes["bin_uri"];
	size_t species_ind     = indexes["species_name"];
	size_t institution_ind = indexes["institution_storing"];
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("BIN index: "<<bin_ind);
		PRINT("Species name index: "<<species_ind);
		PRINT("Institution storing index: "<<institution_ind);
		PRINT("*--------------------------------------*");
	#endif
	data = remove_empty_col(data, species_ind);	
	auto species_names  = unique_elems<std::string_view>(data, species_ind);
	auto bin_names = unique_elems<std::string_view>(data, bin_ind);
	#ifdef DEBUG
		PRINT("*--------------------------------------*");
		PRINT("Unique species names");
		PRINTV(species_names);
		PRINT("*--------------------------------------*");
		PRINT("*--------------------------------------*");
		PRINT("Unique BIN names");
		PRINTV(bin_names);
		PRINT("*--------------------------------------*");
	#endif
	std::vector<Species> out;
	fill(out,species_names);

	size_t count = out.size();	

	for (size_t i = 0; i < count; i++) {
		Species& current = out[i];
		/* It is possible to eliminate this verification */
		
		bool enough = enoughData(current, data, species_ind);
		if (enough){
			bool obin = allSeqs1BIN(data,species_ind,current,bin_ind);
			#ifdef DEBUG
				PRINT("*--------------------------------------*");
				PRINT("Bins for species: "<< current.species_name);
				PRINTV(current.bins);
				PRINT("*--------------------------------------*");
			#endif
			if(obin){
				bool BINSpeciesConcordance = speciesPerBIN(data,current.bins[0],species_ind,bin_ind);
				#ifdef DEBUG
					PRINT("*--------------------------------------*");
					PRINT("Concordance is " << BINSpeciesConcordance <<" for species: "<< current.species_name);
					PRINT("*--------------------------------------*");
				#endif
				if(BINSpeciesConcordance){
					speciesCongruence(data,current.species_name,species_ind,institution_ind);
				}
				else{
					#ifdef DEBUG
						PRINT("*--------------------------------------*");
						PRINT("Species "<< current.species_name << " marked with E1");
						PRINT("*--------------------------------------*");
					#endif
					for(auto & item : data) {
						if (item[species_ind] == current.species_name){
							item.grade = Grade::E1;
						}
					}
				}
			}
			else {
				Grade g = BINnearestNeighb(current.bins, data, bin_ind, species_ind);
				#ifdef DEBUG
					PRINT("*--------------------------------------*");
					PRINT("Species "<< current.species_name << " marked with " << static_cast<typename std::underlying_type<Grade>::type>(g));
					PRINT("*--------------------------------------*");
				#endif
				for(auto& item: data) {
					if( item[species_ind] == current.species_name)
						item.grade = g;
				}
			}
		}
	}


	#ifdef DEBUG
	PRINT("DATA")
	for (size_t i = 0; i < data.size(); i++)
	{
		PRINT(data[i][1] << ";" << static_cast<typename std::underlying_type<Grade>::type>(data[i].grade));
	}
	#endif
}



}