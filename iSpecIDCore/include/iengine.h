#ifndef IENGINE_H
#define IENGINE_H
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "species.h"
#include "record.h"



class IEngine
{
public:

    IEngine();
    void load(std::string filePath);
    void save(std::string filePath);
    template<class Predicate>
    void filter(Predicate pred){
        entries = utils::filter(entries, pred, filteredEntries);
    }
    void group();
    void annotate();

    std::unordered_map<std::string,int> countFilterBadEntries();
    std::vector<int> calculateGradeResults();
    void gradeRecords();

    inline std::vector<Record>& getEntries() { return entries;}
    inline std::vector<Record> getEntriesCopy() { return entries;}
    inline std::vector<Record> getFilteredEntriesCopy() { return filteredEntries;}
    inline void setEntries(std::vector<Record> entries) { this->entries = entries ;}
    inline void setFilteredEntries(std::vector<Record> filteredEntries) { this->filteredEntries = filteredEntries ;}
    inline std::vector<Record>& getFilteredEntries() { return filteredEntries;}
    inline std::unordered_map<std::string, Species> getGroupedEntries() {return groupedEntries;}
    inline int size() {return entries.size();};


private:
    std::vector<std::string> header;
    std::vector<Record> entries;
    std::vector<Record> filteredEntries;
    std::unordered_map<std::string, Species> groupedEntries;
};

#endif // IENGINE_H
