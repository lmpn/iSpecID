#include <utils.hpp>
namespace utils{
	double kbest(const std::vector<double>& vec){
		int best = vec.size() * 0.625;
		std::vector<double> cvec(vec);
		std::sort(cvec.begin(),cvec.end());
		double result = std::accumulate(cvec.begin(), cvec.begin()+best,0.0) / best;
		return result;
	}

}