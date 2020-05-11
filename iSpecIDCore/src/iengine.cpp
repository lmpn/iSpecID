#include <fstream>      // std::ofstream
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "iengine.h"
#include "csv.hpp"
#include "annotator.h"

IEngine::IEngine()
{
    int cores = boost::thread::hardware_concurrency();
    pool = new boost::asio::thread_pool(cores);
}

void IEngine::filter(std::function<bool(const Record&)> pred){
    entries = utils::filter(entries, pred, filtered_entries);
}

void IEngine::load_distance_matrix(std::string filePath){
    dist_matrix.clear();
    csv::CSVFormat format;
    format.delimiter(';');
    csv::CSVReader reader(filePath, format);
    for (auto& row: reader) {
        auto key = row[0].get();
        auto value = std::make_pair(row[1].get(), row[2].get<double>());
        dist_matrix.insert({key, value });
    }
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
            item.update("", "grade");
            item.update("", "modification");
            writer << item.getFields();
        }
        ofs.flush();
        ofs.close();
    }
    else {
        // ifs is bad - deal with it
    }
}





std::string getKey(Record& record){
    return record["species_name"];
};

void join(Species& species, Record& record){
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



void IEngine::annotate(std::vector<std::string> &errors){
    /*
    int tasks = grouped_entries.size();
    std::condition_variable cv;
    std::mutex lock;
    int completed = 0;
    for(auto& pair : grouped_entries){
        auto& species = pair.second;
        boost::asio::post(*pool, [&](){
            annotator::annotateItem(species, grouped_entries, dist_matrix, errors, min_labs, min_dist, min_deposit);
            {
                auto ul = std::unique_lock<std::mutex>(lock);
                completed++;
                if(completed == tasks){
                    cv.notify_one();
                }
            }
        }
        );
    }
    {
        auto ul = std::unique_lock<std::mutex>(lock);
        cv.wait(ul, [&](){return tasks == completed;});
    }*/

    annotator::annotationAlgo(grouped_entries, dist_matrix, errors, min_labs, min_dist, min_deposit);
}

void IEngine::gradeRecords(){
    for(auto& item : entries){
        auto key = item["species_name"];
        auto grade = grouped_entries[key].grade;
        item.update(grade, "grade");
    }
}
