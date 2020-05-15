#ifndef PARSER_H
#define PARSER_H
#include <string>
#include <vector>
#include "csv.hpp"
#include "util.h"
#include <functional>

namespace parse{


template<class T = std::vector<std::string>>
using OutMapper = std::function<std::vector<std::string>(T)>;

template<class T = std::vector<std::string>>
OutMapper<T> getDefaultOutMapper(){
    OutMapper<T> mapper = [](T row){
        return row;
    };
    return mapper;
}


template<class T = std::vector<std::string>>
using InMapper = std::function<T(std::vector<std::string>, csv::CSVRow&)>;

template<class T = std::vector<std::string>>
InMapper<T> getDefaultInMapper(){
    InMapper<T> mapper = [](std::vector<std::string> header, csv::CSVRow& row){
        return row.operator std::__1::vector<std::__1::string, std::__1::allocator<std::__1::string>>();
    };
    return mapper;
}

enum class Format { CSV, TSV};


class Parser
{
public:

    
    Parser(Format file_format = Format::TSV){
        internal_format = file_format;
    }
    ~Parser(){};

    template<class T = std::vector<std::string>>
    std::vector<T> loadFile(std::string file_path, InMapper<T> mapper = getDefaultInMapper()){
        std::vector<T> rows;
        csv::CSVFormat format;
        if(internal_format == Format::CSV){
            format.delimiter(';').header_row(0);
        }
        else if(internal_format == Format::TSV){
            format.delimiter('\t').header_row(0);
        }
        csv::CSVReader reader(file_path, format);
        header = reader.get_col_names();
        if(std::find(header.begin(),header.end(),"grade") == header.end()){
            header.push_back("grade");
        }
        
        if(std::find(header.begin(),header.end(),"modification") == header.end()){
            header.push_back("modification");
        }

        for (auto& row: reader) {
            T record = mapper(header,row);
            rows.push_back(record);
        }
        return rows;
    }
    
    template<class T = std::vector<std::string>>
    std::vector<T> loadFile(std::string file_path, std::vector<std::string>& header, InMapper<T> mapper = getDefaultInMapper()){
        std::vector<T> rows;
        csv::CSVFormat format;
        if(internal_format == Format::CSV){
            format.delimiter(';').header_row(0);
        }
        else if(internal_format == Format::TSV){
            format.delimiter('\t').header_row(0);
        }
        csv::CSVReader reader(file_path, format);
        this->header = header;

        for (auto& row: reader) {
            T record = mapper(header, row);
            rows.push_back(record);
        }
        return rows;
    }

    template<class T = std::vector<std::string>>
    void saveCSVFile(std::string file_path, std::vector<std::string>& header, std::vector<T>& rows, OutMapper<T> mapper = getDefaultOutMapper()){
        std::ofstream ofs (file_path, std::ofstream::out|std::ofstream::binary);
        if ( ofs ) {
            auto writer = csv::make_csv_writer(ofs);
            writer << header;
            for(auto& row : rows){
                writer << mapper(row);
            }
            ofs.flush();
            ofs.close();
        }
        else {}
    }

    template<class T = std::vector<std::string>>
    void saveTSVFile(std::string file_path, std::vector<std::string>& header, std::vector<T>& rows, OutMapper<T> mapper = getDefaultOutMapper()){
        std::ofstream ofs (file_path, std::ofstream::out|std::ofstream::binary);
        if ( ofs ) {
            auto writer = csv::make_tsv_writer(ofs);
            writer << header;
            for(auto& row : rows){
                writer << mapper(row);
            }
            ofs.flush();
            ofs.close();
        }
        else {}
    }

    template<class T = std::vector<std::string>>
    void saveFile(std::string file_path, std::vector<std::string>& header, std::vector<T>& rows, Format out_format = Format::TSV, OutMapper<T> mapper = getDefaultOutMapper()){
            if(out_format == Format::TSV){
                saveTSVFile(file_path, header, rows, mapper);
            }
            else if(out_format == Format::CSV){
                saveCSVFile(file_path, header, rows, mapper);
            }
    }


    std::vector<std::string> getHeader(){ return header; };

private:
    Format internal_format;
    std::vector<std::string> header;
};
} // parse
#endif // IENGINE_H
