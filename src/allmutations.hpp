#ifndef ALLMUTATIONS_HPP
#define ALLMUTATIONS_HPP

#include <vector>
#include <random>
#include "types.hpp"
#include "randomcore.hpp"
#include "sequence.hpp"
#include "host.hpp"

class AllMutations{
public:
	using SeqType = Sequence<29903>;
private:
	RNGcore * rng_;
	double mr_;
	Vec<SeqType> seqs_;
	Vec<double> phi_h_;
	std::gamma_distribution<> mut_period_;
public:
	const SeqType& getSequence(unsigned i) const;
	double getMutationRate() const;
	double getPhiH(unsigned i) const;
	void setRNG(RNGcore * rng);
	void setMutationRate(double mr);
	unsigned newMutation();
	Time nextMutation();
};


const AllMutations::SeqType& AllMutations::getSequence(unsigned i) const{
	return seqs_[i];
}

double AllMutations::getMutationRate() const{
	return mr_;
}

double AllMutations::getPhiH(unsigned i) const{
	return phi_h_[i];
}

void AllMutations::setRNG(RNGcore * rng){
	rng_ = rng;
}

void AllMutations::setMutationRate(double mr){
	mr_ = mr;
	// gamma distribution arbitrarily chosen
	// parametrized with: mean = k * theta, and: variance = k * theta^2
	double k = 10; // arbitrarily chosen to preserve the shape
	double theta = 1 / mr_ / k;
	mut_period_ = std::gamma_distribution<>(k, theta);
}

unsigned AllMutations::newMutation(){
	seqs_.emplace_back(SeqType());
	phi_h_.emplace_back(1);
	return seqs_.size() - 1;
}

Time AllMutations::nextMutation(){
	return mut_period_(rng_->get());
}

#endif