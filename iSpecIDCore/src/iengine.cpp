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




void IEngine::load(std::string filePath){
    clear();
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

void IEngine::saveDistanceMatrix(std::string filePath){
    std::ofstream ofs (filePath, std::ofstream::out|std::ofstream::binary);
    if ( ofs ) {
        // ifs is good
        auto writer = csv::make_csv_writer(ofs);
        for(auto& item : dist_matrix){
            auto bin_a = item.first;
            auto bin_b = item.second.first;
            auto dist = item.second.second;
            std::vector<std::string> row({bin_a, bin_b, std::to_string(dist)});
            writer << row;
        }
        ofs.flush();
        ofs.close();
    }
    else {
        // ifs is bad - deal with it
    }
}



void IEngine::filter(std::function<bool(const Record&)> pred){
    entries = utils::filter(entries, pred, filtered_entries);
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


/*
    TODO: Receive function to get data
*/
void IEngine::populateDistanceMatrix(){
    std::vector<std::string> errors;
    std::unordered_set<std::string> unique_bins;
    if(grouped_entries.size() > 0){
        for(auto& entry : grouped_entries){
            auto& bins = entry.second.bins;
            for(auto& bin_entry: bins){
                unique_bins.insert(bin_entry.first);
            }
        }
    }else if(entries.size() > 0){
        for(auto& entry: entries){
            auto bin_name = entry.operator[]("bin_uri");
            unique_bins.insert(bin_name);
        }
    }    
    
    for(auto bin : unique_bins){
        boost::asio::post(*pool, [&,bin](){
            try{
                auto bold_data = annotator::parseBoldData(bin, errors);
                dist_matrix.insert({bin, std::make_pair<>(bold_data.neighbour, bold_data.distance)});
            }catch(std::exception &e){
                std::cout << e.what() << std::endl;
            }
        });
    }
}


/*
    TODO: Receive function annotate species
*/
void IEngine::annotate(std::vector<std::string> &errors){

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
    }

    //annotator::annotationAlgo(grouped_entries, dist_matrix, errors, min_labs, min_dist, min_deposit);
}

void IEngine::gradeRecords(){
    for(auto& item : entries){
        auto key = item["species_name"];
        auto grade = grouped_entries[key].grade;
        item.update(grade, "grade");
    }
}
