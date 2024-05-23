#ifndef MUTACTIVE_HPP
#define MUTACTIVE_HPP

#include <algorithm>
#include <random>
#include "../../types.hpp"
#include "../../host.hpp"
#include "../../haplotypes.hpp"
#include "../../randomcore.hpp"
#include "../../algorithms.hpp"
#include "mutdiff.hpp"

class MutActive{
private:
	Haplotypes * haplos_;
	const PatchID patch_id_;
	const double rate_;
	Vec<Vec<Host>> hosts_;
public:
	friend class MutDiff;
	MutActive(PatchID patch_id, unsigned gamma_trick, double rate);
	void setHaplotypes(Haplotypes * seqdealer);
	unsigned size() const;
	double getPhi() const;
	Vec<Vec<Host>>& getHosts();
	void shift(RNGcore * rng);
	MutDiff getNewErased(RNGcore * rng) const;
	MutDiff sampleInfectors(RNGcore * rng, unsigned n) const;
	void updateHaplotypes(Time t);
};

MutActive::MutActive(PatchID patch_id, unsigned gamma_trick, double rate) : patch_id_(patch_id), rate_(rate), hosts_(gamma_trick) {}

void MutActive::setHaplotypes(Haplotypes * seqdealer){
	haplos_ = seqdealer;
}

unsigned MutActive::size() const{
	unsigned size = 0;
	for (auto & h : hosts_){
		size += h.size();
	}
	return size;
}

double MutActive::getPhi() const{
	unsigned phi = 0;
	for (auto & hosts : hosts_){
		for (auto & h : hosts){
			phi += haplos_->getPhiH(h.evolved_mut_);
		}
	}
	return phi;
}

Vec<Vec<Host>>& MutActive::getHosts(){
	return hosts_;
}

void MutActive::shift(RNGcore * rng){
	if (hosts_.size() > 1){
		for (int i = hosts_.size()-2; i >= 0; --i){
			std::binomial_distribution Distr(hosts_[i].size(), rate_);
			unsigned n = Distr(*rng);
			Vec<unsigned> indices = sampleIndices(rng->get(), hosts_[i].size(), n);
			appendToEraseFromByIndices(hosts_[i+1], hosts_[i], indices);
		}
	}
}

MutDiff MutActive::getNewErased(RNGcore * rng) const{
	unsigned size = hosts_.back().size();
	std::binomial_distribution Distr(size, rate_);
	unsigned n = Distr(*rng);
	Vec<unsigned> indices = sampleIndices(rng->get(), size, n);
	return MutDiff(patch_id_, indices);
}

MutDiff MutActive::sampleInfectors(RNGcore * rng, unsigned n) const{
	unsigned tot_hosts = 0;
	for (auto & h : hosts_){
		tot_hosts += h.size();
	}
	Vec<double> weights(tot_hosts);
	unsigned idx = 0;
	for (auto & hosts : hosts_){
		for (auto & h : hosts){
			weights[idx] = haplos_->getPhiH(h.evolved_mut_);
			++idx;
		}
	}
	Vec<unsigned> indices = sampleIndicesWithReplacement(rng->get(), weights, n);
	std::sort(indices.begin(), indices.end());
	Vec<Host> sampled(n);
	unsigned compartment = 0;
	unsigned prev_occupants = 0;
	for (unsigned i = 0; i < n; ++i){
		while (indices[i] >= prev_occupants + hosts_[compartment].size()){
			prev_occupants += hosts_[compartment].size();
			++compartment;
		}
		sampled[i] = hosts_[compartment][indices[i]-prev_occupants];
	}
	std::shuffle(sampled.begin(), sampled.end(), rng->get());
	return MutDiff(patch_id_, sampled);
}

void MutActive::updateHaplotypes(Time t){
	for (auto & hosts : hosts_){
		for (auto & infectious : hosts){
			if (t >= infectious.t_next_mut_){
				Time tnext = t + haplos_->nextMutation();
				unsigned newmut = haplos_->newMutation(infectious.evolved_mut_);
				infectious.evolveMutation(t, newmut, tnext);
			}
		}
	}
}

#endif