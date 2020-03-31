#include <utils.h>
namespace utils{
    double kbest(const std::vector<double>& vec){
        int best = vec.size() * 0.625;
        std::vector<double> cvec(vec);
        std::sort(cvec.begin(),cvec.end());
        double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
        return result;
    }

    std::vector<std::string> split(const std::string& str, const std::string& delims)
    {
        std::vector<std::string> output;
        //output.reserve(str.size() / 2);

        for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1) {
            second = std::find_first_of(first, last, std::begin(delims), std::end(delims));

                output.emplace_back(first, second);
        }

        return output;
    }
}
