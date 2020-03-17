#ifndef RECORD_HPP
#define RECORD_HPP
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <utils.hpp>

class Record{
    private:
    std::shared_ptr<std::unordered_map<std::string, size_t>> indexes;
    std::string data;
    std::vector<std::string> fields;
    
    public:
    Record() = default;

    Record(std::string _data){
        data = _data;
        boost::split(fields,data,[](char delim){return delim == '\t';});
    };

    Record(std::string _data, std::shared_ptr<std::unordered_map<std::string, size_t>> _indexes): indexes(_indexes) {
        data = _data;
        boost::split(fields,data,[](char delim){return delim == '\t';});
    };

    bool empty() const;
    size_t size() const;
    std::string operator[](std::string name);
    std::string operator[](size_t index);
    void update(std::string field, std::string name);
    void update(std::string field, size_t index);
    friend std::ostream & operator << (std::ostream &strm, Record obj) {
        strm << "data:" <<std::endl;
        for(auto& f : obj.fields)
            strm << f << ",";
        strm <<std::endl;
        return strm;
    }


    
    class Iterator; 
    Iterator begin();
    Iterator end();

    class Iterator 
    { 
        public:
            Iterator(Record* _record) noexcept : it (_record->fields.begin()) { };
            Iterator& operator=(Record* it);
            Iterator& operator++();
            Iterator& operator--();
            bool operator!=(const Iterator& iterator);
            std::string_view operator*();
        private: 
            std::vector<std::string>::iterator it;
    };
};
#endif 