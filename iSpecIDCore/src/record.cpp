#include <record.h>



bool Record::empty() const { return this->fields.empty(); }

size_t Record::size() const { return this->fields.size(); }



std::string Record::operator[](std::string name) const{
    try{
        auto index = indexes.get()->operator[](name);
        return this->operator[](index);
    }catch(std::out_of_range e){
        return this->operator[](this->size());
    }
}

std::string Record::operator[](size_t index) const{
    return fields.at(index);
}

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

Record::Iterator Record::begin()
{
    return Iterator(this);
}

Record::Iterator Record::end()
{
    return Iterator(nullptr);
}




Record::Iterator& Record::Iterator::operator=(Record* it)
{
    this->it = it->fields.begin();
    return *this;
}

// Prefix ++ overload
Record::Iterator& Record::Iterator::operator++()
{
    it++;
    return *this;
}



Record::Iterator& Record::Iterator::operator--()
{
    it--;
    return *this;
}


bool Record::Iterator::operator!=(const Iterator& iterator)
{
    return iterator.it != it;
}

std::string_view Record::Iterator::operator*()
{
    return *it;
}
