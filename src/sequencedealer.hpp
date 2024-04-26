#ifndef SEQUENCEDEALER_HPP
#define SEQUENCEDEALER_HPP

#include <vector>
#include <random>
#include "types.hpp"
#include "randomcore.hpp"
#include "sequence.hpp"
#include "host.hpp"


class SequenceDealer{
public:
	static constexpr unsigned LEN = 32000;
	using SeqType = Sequence<LEN>;
	struct IdSeqType{
		using SLEN = char[LEN+1];
		unsigned id;
		SLEN s;
	};
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
	unsigned newMutation(unsigned i);
	Time nextMutation();
	np_array<IdSeqType> read() const;
};

const SequenceDealer::SeqType& SequenceDealer::getSequence(unsigned i) const{
	return seqs_[i];
}

double SequenceDealer::getMutationRate() const{
	return mr_;
}

double SequenceDealer::getPhiH(unsigned i) const{
	return phi_h_[i];
}

void SequenceDealer::setRNG(RNGcore * rng){
	rng_ = rng;
}

void SequenceDealer::setMutationRate(double mr){
	mr_ = mr;
	// gamma distribution arbitrarily chosen
	// parametrized with: mean = k * theta, and: variance = k * theta^2
	double k = 10; // arbitrarily chosen to preserve the shape
	double theta = 1 / mr_ / k;
	mut_period_ = std::gamma_distribution<>(k, theta);
}

unsigned SequenceDealer::newMutation(unsigned i){
	if (i == -1)	{	seqs_.emplace_back(SeqType(rng_));	}
	else			{	seqs_.emplace_back(seqs_[i].generateMutation(rng_));	}
	phi_h_.emplace_back(1);
	return seqs_.size() - 1;
}

Time SequenceDealer::nextMutation(){
	return mut_period_(rng_->get());
}

np_array<SequenceDealer::IdSeqType> SequenceDealer::read() const{
	np_array<IdSeqType> array(seqs_.size());
	auto view = array.mutable_unchecked<1>();
	for (unsigned i = 0; i < seqs_.size(); ++i){
		view[i].id = i;
		seqs_[i].writeSequenceInto(view[i].s);
	}
	return array;
}

#endif