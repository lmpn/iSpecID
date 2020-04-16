#ifndef RECORD_H
#define RECORD_H
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>

class Record{
    private:
    std::vector<std::string> fields;
    std::shared_ptr<std::unordered_map<std::string, size_t>> indexes;

    public:
    Record() = default;

    Record( std::vector<std::string> fields, std::shared_ptr<std::unordered_map<std::string, size_t>> indexes):
        fields(fields), indexes(indexes)
    {
        if(indexes->size() > fields.size()){
            this->fields.push_back("U");
            this->fields.push_back("");
        }
    };

    std::vector<std::string>& getFields(){ return fields;}
    bool empty() const;
    size_t size() const;
    std::string operator[](std::string name) const;
    std::string operator[](size_t index) const;
    void update(std::string field, std::string name);
    void update(std::string field, size_t index);
    friend std::ostream & operator << (std::ostream &strm, Record obj) {
        strm << "data:" <<std::endl;
        for(auto& f : obj.fields)
            strm << f << ",";
        strm <<std::endl;
        return strm;
    }
};
#endif
