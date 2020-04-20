#include <fstream>      // std::ofstream
#include "iengine.h"
#include "utils.h"
#include "csv.hpp"
#include "annotator.h"

IEngine::IEngine()
{

}

void IEngine::load(std::string filePath){
    entries.clear();
    filtered_entries.clear();
    grouped_entries.clear();


    csv::CSVFormat format;
    format.delimiter('\t').header_row(0);
    csv::CSVReader reader(filePath, format);
    header = reader.get_col_names();
    if(std::find(header.begin(),header.end(),"grade") == header.end()){
        header.push_back("grade");
    }
    if(std::find(header.begin(),header.end(),"modification") == header.end()){
        header.push_back("modification");
    }
    auto indexes = utils::createIndexedHeader(header);
    for (auto& row: reader) {
        Record record(row.operator std::vector<std::string>(), indexes);
        entries.push_back(record);
    }
}


void IEngine::save(std::string filePath){
    std::ofstream ofs (filePath, std::ofstream::out|std::ofstream::binary);
    if ( ofs ) {
        // ifs is good
        auto writer = csv::make_tsv_writer(ofs);
        writer << header;
        for(auto& item : entries){
            writer << item.getFields();
        }
        for(auto& item : filtered_entries){
            writer << item.getFields();
        }
        ofs.flush();
        ofs.close();
    }
    else {
        // ifs is bad - deal with it
    }
}





std::string getKey(Record record){
    return record["species_name"];
};

void join(Species& species, Record record){
    species.push_back(record);
};

void IEngine::group(){
    grouped_entries = utils::group(entries, getKey, join);
}


std::unordered_map<std::string,int> IEngine::countFilterBadEntries(){
    size_t species = 0;
    size_t bin = 0;
    size_t institution = 0;
    std::vector<Record> tmp;
    for(auto& item : entries){
        if(item["species_name"].empty()){
            species++;
            filtered_entries.push_back(item);
        }
        else if(item["bin_uri"].empty()){
            bin++;
            filtered_entries.push_back(item);
        }
        else if(item["institution_storing"].empty()){
            institution++;
            filtered_entries.push_back(item);
        }else{
            tmp.push_back(item);
        }
    }
    entries.swap(tmp);
    return {{"species", species},{"bin", bin},{"institution", institution}};
}

std::vector<int> IEngine::calculateGradeResults(){
    auto indexE1 = 4;
    std::vector<int> counts(indexE1+2,0);
    for(auto& item : entries){
        auto grade = item["grade"];
        if( grade == "E1" || grade == "E2")
        {
            counts[indexE1] ++;
        }
        else{
            auto index = grade[0] - 'A';
            counts[index]++;
        }
    }
    counts.at(5) = entries.size();
    return counts;
}



void IEngine::annotate(){
    annotator::annotationAlgo(grouped_entries);
}

void IEngine::gradeRecords(){
    for(auto& item : entries){
        auto key = item["species_name"];
        auto grade = grouped_entries[key].grade;
        item.update(grade, "grade");
    }
}
