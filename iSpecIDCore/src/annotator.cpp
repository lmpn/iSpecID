#include <annotator.h>




void Annotator::annotationAlgo(){
    for(auto& pair : this->group_records){
        auto& species = pair.second;
        std::string grade = "D";
        if(species.specimens.size() > 3){
            grade = "E1";
            if(species.bins.size() == 1){
                bool BINSpeciesConcordance = speciesPerBIN(*species.bins.begin());
                if(BINSpeciesConcordance){
                    grade = species.institution.size()==1 ? "B" : "A";
                }
            }else{
                grade = findBinsNeighbour(species.bins);
            }
            species.grade = grade;
        }else{
            species.grade = grade;
        }
    }
}

/*
 * Getters
*/
std::vector<Record> Annotator::getRecords(){
    return this->records;
}
umap<std::string, Species> Annotator::getGroupRecords(){
    return this->group_records;
}

std::vector<int> Annotator::getGradeResults() {
    return results;
}


/*
 * Post-processing functions
*/

void Annotator::calculateGradeResults(){
    auto indexE1 = 4;
    std::vector<int> counts(indexE1+2,0);
    for(auto& pair : group_records){
        auto grade = pair.second.grade;
        size_t num = pair.second.specimens.size();
        if( grade == "E1" || grade == "E2")
        {
            counts[indexE1] += num;
        }
        else{
            auto index = grade[0] - 'A';
            counts[index] += num;
        }
    }
    counts.at(5) = records.size();
    results = counts;
}

void Annotator::gradeRecords(){
    for(auto& item : this->records){
        auto key = item["species_name"];
        auto grade = group_records[key].grade;
        item.update(grade, "grade");
    }

}


/*
 * Pre-processing functions
*/
void Annotator::clear(){
    if(size() > 0){
        results.clear();
        records.clear();
        group_records.clear();
    }
}


void Annotator::cleanLoad(std::string file_path){
    clear();
    load(file_path);
    filter([](Record item) {return item["species_name"].empty();});
    group();
}

void Annotator::load(std::string file_path){
    Reader<Record> reader(file_path);
    this->records = reader.get_records();
}

void Annotator::clearGroup(){
    for(auto& pair : group_records){
        pair.second.bins.clear();
        pair.second.specimens.clear();
        pair.second.institution.clear();
    }
}

void Annotator::group(){
    for (auto& specimen : this->records)
    {
        auto key = specimen["species_name"];
        auto& current = this->group_records[key];
        current.push_back(specimen);
    }
}




/*
 * Core Algorithm Helpers
*/

bool Annotator::speciesPerBIN(std::string bin){
    std::unordered_set<std::string_view> unique_set;
    size_t count = 0;
    for(auto & entry : group_records){
        auto fst = entry.second.bins.begin();
        auto lst = entry.second.bins.end();
        auto result = std::find( fst, lst, bin);
        if(lst != result && (++count) == 2){
            return false;
        }
    }
    return true;
}

BoldData Annotator::parseBoldData(std::string bin){
    BoldData bd;
    bd.distance = std::numeric_limits<int>::max();
    bd.neighbour = "";
    Miner mn;
    std::string url("http://v4.boldsystems.org/index.php/Public_BarcodeCluster?clusteruri=" + std::string(bin));
    try{
        std::string s = mn.getPage(url.c_str());
        std::smatch matches;
        std::regex_search (s,matches,all);
        auto next = matches.suffix().str();
        float d = std::stof(matches[1]);
        std::regex_search (next,matches,all);
        std::string nbin = matches[2];
        bd.distance = d;
        bd.neighbour = nbin;
    }catch (const std::exception& e) {
        PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return bd;
}



std::string Annotator::findBinsNeighbour(uset<std::string> bins)
{
    auto grade = "E2";
    size_t count = records.size();
    size_t ctr = 0;
    for(auto& pair : group_records){
        auto cur_bins = pair.second.bins;
        if(utils::hasIntersection(bins,cur_bins)){
            ctr+=1;
            if(ctr>1) return grade;
        }
    }
    count = bins.size();
    auto it = bins.begin();
    ugraph graph(count);
    for(size_t  i= 0; i < count; i++){
        auto bold = parseBoldData(*it);
        auto ind  = utils::findIndex(std::begin(bins), std::end(bins), bold.neighbour);
        if( ind != -1 && bold.distance <=2) {
            boost::add_edge(i, ind, bold.distance, graph);
            boost::add_edge(ind, i, bold.distance, graph);
        }
        it++;
    }
    std::vector<int> component (count);
    size_t num_components = boost::connected_components (graph, &component[0]);
    if( num_components == 1){
        grade="C";
    }
    return grade;
}
