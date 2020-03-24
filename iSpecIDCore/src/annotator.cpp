#include <annotator.h>



void Annotator::annotate(std::string file_path){
    this->load(file_path);
    this->filter([](Record item) {return item["species_name"].empty();});
    this->group();
    this->annotation_algo();
}

void Annotator::annotation_algo(){
    for(auto& pair : this->group_data){
        auto& species = pair.second;
        utils::Grade grade = utils::Grade::D;
        if(species.specimens.size() > 3){
            utils::Grade grade = utils::Grade::E1;
            if(species.bins.size() == 1){
                bool BINSpeciesConcordance = speciesPerBIN(*species.bins.begin());
                if(BINSpeciesConcordance){
                    grade = species.institution.size()==1 ? utils::Grade::B : utils::Grade::A;
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

void Annotator::load(std::string file_path){
    Reader<Record> reader(file_path);
    this->data = reader.get_records();
}


void Annotator::group(){
    for (auto& specimen : this->data)
    {
        auto key = specimen["species_name"];
        auto& current = this->group_data[key];
        current.push_back(specimen);
    }
}


std::vector<Record> Annotator::getData(){
    return this->data;
}
umap<std::string, Species> Annotator::getGroupData(){
    return this->group_data;
}


std::vector<int> Annotator::getGradeResults()
{
    auto indexE1 = static_cast<typename std::underlying_type<utils::Grade>::type>(utils::Grade::E1);
    std::vector<int> results(indexE1+1);
    for(auto& pair : group_data){
        utils::Grade grade = pair.second.grade;
        size_t num = pair.second.specimens.size();
        if(grade == utils::Grade::E1 || grade == utils::Grade::E2)
        {
            results[indexE1] += num;
        }
        else{
            auto index = static_cast<typename std::underlying_type<utils::Grade>::type>(grade);
            results[index] += num;
        }
    }
    return results;
}

bool Annotator::speciesPerBIN(std::string bin){
    std::unordered_set<std::string_view> unique_set;
    size_t count = 0;
    for(auto & entry : group_data){
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
        std::regex dist("Distance to Nearest Neighbor:</th>\\s*<td>(\\d+\\.\\d+)%");
        std::regex nearest("Nearest BIN URI:</th>\\s*<td>(.*)</td>");
        std::regex_search (s,matches,nearest);
        std::string nbin = matches[1];
        std::regex_search (s,matches,dist);
        float d = std::stof(matches[1]);
        bd.distance = d;
        bd.neighbour = nbin;
    }catch (const std::exception& e) {
        PRINT("Error fetching URL: " << url <<std::endl << "Reason: "<< e.what());
    }
    return bd;
}



utils::Grade Annotator::findBinsNeighbour(uset<std::string> bins)
{
    utils::Grade grade = utils::Grade::E2;
    size_t count = data.size();
    size_t ctr = 0;
    for(auto& pair : group_data){
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
        grade=utils::Grade::C;
    }
    return grade;
}
