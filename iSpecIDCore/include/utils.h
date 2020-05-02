#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
#include <iterator>
#include <memory>

#define PRINT(X) std::cout << X << std::endl;
#define PRINTV(X) for(auto x : X){ std::cout << x << " "; }  std::cout << std::endl;
namespace utils{
    enum class Grade{
        A, B, C, D, E1, E2, U
    };


    template <class T>
    T argParse(int argc, char *argv[], std::string prefix, T defaultValue)
    {
        for (int i = 0; i < argc; ++i){
            std::string argument(argv[i]);
            if(argument.substr(0, prefix.size()) == prefix) {
                std::string argument_value = argument.substr(prefix.size());
                std::istringstream ss(argument_value);
                T value;
                ss >> value;
                return value;
            }
        }
        return defaultValue;
    };

    template <class _ForwardIterator, class _Tp>
    int findIndex(_ForwardIterator __first, _ForwardIterator __last, const _Tp &__value)
    {
        auto elem = std::find(__first, __last, __value);
        auto dist = std::distance(__first, elem);
        auto size = std::distance(__first, __last);
        return dist < size ? dist : -1;
    };


    template<class K, class V>
    bool hasIntersection(std::unordered_map<K,V>& o1, std::unordered_map<K,V>& o2){
        size_t o1s = o1.size();
        size_t o2s = o2.size();
        if(o1s <= o2s){
            auto o2e = o2.end();
            for (auto item : o1){ //= o1.begin(); i != o1.end(); i++) {
                if(o2.find(item.first) != o2e) return true;
            }
        }else{
            auto o1e = o1.end();
            for (auto item : o2){ //= o1.begin(); i != o1.end(); i++) {
                if(o1.find(item.first) != o1e) return true;
            }
        }
        return false;
    }


    std::shared_ptr<std::unordered_map<std::string, size_t>>  createIndexedHeader(std::vector<std::string> header);

    double kbest(const std::vector<double>& vec);
    std::vector<std::string> split(const std::string& str, const std::string& delims = "\t");



    template<class Key, class Value>
    using Aggregation  = std::unordered_map<Key, Value>;

    template<class Item>
    using Table = std::vector<Item>;


    template<class Item, class Predicate>
    Table<Item> filter(Table<Item>& tbl, Predicate pred, Table<Item>& filtered = {}){
        Table<Item> accept;
        for(auto& item : tbl){
            if(!pred(item)){
                accept.push_back(std::move(item));
            }else{
                filtered.push_back(std::move(item));
            }
        }
        return accept;
    }

    template<class Item, class Key, class Value>
    Aggregation<Key, Value> group(Table<Item>& tbl, Key get_key(Item), void join_op(Value&, Item)){
        Aggregation<Key, Value> result;
        for (auto& item : tbl)
        {
            Key item_key = get_key(item);
            Value& current_value = result[item_key];
            join_op(current_value, item);
        }
        return result;
    }
}
#endif
