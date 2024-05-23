#ifndef DISTRIBUTIONS_HPP
#define DISTRIBUTIONS_HPP

#include <random>

class GammaDistribution{
	std::gamma_distribution<> distr_;
	double x0;
public:
	GammaDistribution() : distr_(), x0(0) {};
	GammaDistribution(double k, double theta, double x0=0) : distr_(k, theta), x0(x0) {};
	template<typename RndEng>
	double operator()(RndEng & eng){
		return distr_(eng) + x0;
	}
};

#endif