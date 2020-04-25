#include "record.h"



bool Record::empty() const { return this->fields.empty(); }

size_t Record::size() const { return this->fields.size(); }





void Record::update(std::string field, std::string name){
    try{
        auto index = indexes.get()->at(name);
        this->update(field, index);
    }catch(std::out_of_range e){
        this->update(field, this->size());
    }
}


void Record::update(std::string field, size_t index){
    fields.at(index) = field;
}
