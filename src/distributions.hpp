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
		return distr_(eng) - x0;
	}
};

class BetaDistribution{
	std::gamma_distribution<> distr_a_;
	std::gamma_distribution<> distr_b_;
	double x0_;
	double scale_;
public:
	BetaDistribution() : distr_a_(), distr_b_(), x0_(0), scale_(1) {};
	BetaDistribution(double alpha, double beta, double x0=0, double scale=1) : distr_a_(alpha, 1), distr_b_(beta, 1), x0_(x0), scale_(scale) {};
	template<typename RndEng>
	double operator()(RndEng & eng){
		double a = distr_a_(eng);
		double b = distr_b_(eng);
		return a / (a + b) * scale_ - x0_;
	}
};

#endif