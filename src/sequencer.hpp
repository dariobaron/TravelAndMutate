#ifndef SEQUENCER_HPP
#define SEQUENCER_HPP

#include <random>
#include "types.hpp"
#include "randomcore.hpp"
#include "host.hpp"


class Sequencer{
	RNGcore * rng_;
	std::bernoulli_distribution distr_;
	Time t_;
	Time delay_;
	Vec<SequencingRecord> sampled_;
public:
	Sequencer(RNGcore * rng, double p, Time delay);
	void update(Time t);
	bool toBeSequenced();
	Time extractSamplingTime();
	void record(const Host & host);
	void recordOnHealing(const Host & host);
	np_array<SequencingRecord> getSampledIDs() const;
};


Sequencer::Sequencer(RNGcore * rng, double p, Time delay) : rng_(rng), distr_(p), delay_(delay) {}


void Sequencer::update(Time t){
	t_ = t;
}


bool Sequencer::toBeSequenced(){
	return distr_(rng_->get());
}


Time Sequencer::extractSamplingTime(){
	return t_ + delay_;
}


void Sequencer::record(const Host & host){
	if (host.t_sequencing_ == t_){
		sampled_.emplace_back(t_, host.patch_, host.evolved_mut_);
	}
}


void Sequencer::recordOnHealing(const Host & host){
	if ((host.t_sequencing_ != -1) && (host.t_sequencing_ > t_)){
		sampled_.emplace_back(t_, host.patch_, host.evolved_mut_);
	}
}



np_array<SequencingRecord> Sequencer::getSampledIDs() const{
	return np_array<SequencingRecord>(sampled_.size(), sampled_.data());
}


#endif