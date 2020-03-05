#ifndef UTILS_HPP
#define UTILS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#define PRINT(X) std::cout << X << std::endl;
#define PRINTV(X) for(auto x : X){ std::cout << x << " "; }  std::cout << std::endl;
namespace utils{
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
    }
    double kbest(const std::vector<double>& vec);
}
#endif