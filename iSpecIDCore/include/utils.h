#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <unordered_set>
#include <type_traits>

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


    template<class T>
    bool hasIntersection(std::unordered_set<T>& o1, std::unordered_set<T>& o2){
        size_t o1s = o1.size();
        size_t o2s = o2.size();
        if(o1s <= o2s){
            auto o2e = o2.end();
            for (auto i = o1.begin(); i != o1.end(); i++) {
                if (o2.find(*i) != o2e) return true;
            }
        }else{
            auto o1e = o1.end();
            for (auto i = o2.begin(); i != o2.end(); i++) {
                if (o1.find(*i) != o1e) return true;
            }
        }
        return false;
    }




    double kbest(const std::vector<double>& vec);
    std::vector<std::string_view> split(std::string_view sv, char delim);
}
#endif
