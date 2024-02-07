#ifndef PATCH_HPP
#define PATCH_HPP

#include "types.hpp"
#include "randomcore.hpp"
#include "recorder.hpp"
#include "haplotype.hpp"

class Patch{
	RNGcore * rng_;
	Recorder rec_;
	unsigned N_;
	unsigned S_, E_, I_, R_;
	double beta_;
	double epsilon_;
	double mu_;
	unsigned Enew_, Inew_, Rnew_;
	bool uninitialized_;
public:
	Patch();
	double getRho() const;
	const Recorder & getRecorder() const;
	void setProperties(RNGcore * rng, unsigned N, double beta, double epsilon, double mu);
	void seed(unsigned I0);
	Vec<unsigned> computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const;
	unsigned sampleInfectors(unsigned Enew) const;
	void addNewInfections(unsigned Enew);
	void setNewRecoveries();
	void setNewOnsets();
	void update(Time t);
	bool isEpidemicAlive() const;
};

Patch::Patch() : uninitialized_(true){}

double Patch::getRho() const{
	return ((double)(I_)) / N_;
}

const Recorder & Patch::getRecorder() const{
	return rec_;
}

void Patch::setProperties(RNGcore * rng, unsigned N, double beta, double epsilon, double mu){
	uninitialized_ = false;
	rng_ = rng;
	N_ = N;
	S_ = N_; E_ = 0; I_ = 0; R_ = 0;
	Enew_ = 0; Inew_ = 0; Rnew_ = 0;
	beta_ = beta;
	epsilon_ = epsilon;
	mu_ = mu;
}

void Patch::seed(unsigned I0){
	S_ -= I0;
	E_ += I0;
}

Vec<unsigned> Patch::computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const{
	Vec<double> probs(rhos.size());
	std::transform(rhos.begin(), rhos.end(), c_ij.begin(), probs.begin(), std::multiplies<>());
	double f = beta_ * std::accumulate(probs.begin(), probs.end(), 0.);
	std::binomial_distribution Binom(S_, f);
	unsigned Enew = Binom(*rng_);
	Vec<unsigned> Ninfectors(rhos.size(), 0);
	std::discrete_distribution Distr(probs.begin(), probs.end());
	for (unsigned i = 0; i < Enew; ++i){
		++Ninfectors[Distr(*rng_)];
	}
	return Ninfectors;
}

unsigned Patch::sampleInfectors(unsigned Enew) const{
	return Enew;
}

void Patch::addNewInfections(unsigned Enew){
	Enew_ += Enew;
}

void Patch::setNewRecoveries(){
	std::poisson_distribution Distr(mu_ * I_);
	Rnew_ = Distr(*rng_);
	Rnew_ = std::min(I_, Rnew_);
}

void Patch::setNewOnsets(){
	std::poisson_distribution Distr(epsilon_ * E_);
	Inew_ = Distr(*rng_);
	Inew_ = std::min(E_, Inew_);
}

void Patch::update(Time t){
	S_ += - Enew_;
	E_ += Enew_ - Inew_;
	I_ += Inew_ - Rnew_;
	R_ += Rnew_;
	rec_.push_trajectory(t, S_, E_, I_, R_, Enew_, Inew_);
	Enew_ = 0;
	Inew_ = 0;
	Rnew_ = 0;
}

bool Patch::isEpidemicAlive() const{
	return E_ + I_;
}

#endif