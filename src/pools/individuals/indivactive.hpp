#ifndef INDIVACTIVE_HPP
#define INDIVACTIVE_HPP

#include <algorithm>
#include <random>
#include "../../types.hpp"
#include "../../individual.hpp"
#include "../../randomcore.hpp"
#include "mixlightlib/algorithms.hpp"
#include "indivdiff.hpp"

class IndivActive{
private:
	const PatchID patch_id_;
	const double rate_;
	Vec<Vec<Individual>> individuals_;
public:
	friend class IndivDiff;
	IndivActive(PatchID patch_id, unsigned gamma_trick, double rate);
	unsigned size() const;
	double getPhi() const;
	void shift(RNGcore * rng);
	IndivDiff getNewErased(RNGcore * rng) const;
	IndivDiff sampleInfectors(RNGcore * rng, unsigned n) const;
};

IndivActive::IndivActive(PatchID patch_id, unsigned gamma_trick, double rate) : patch_id_(patch_id), rate_(rate), individuals_(gamma_trick) {}

unsigned IndivActive::size() const{
	unsigned size = 0;
	for (auto & i : individuals_){
		size += i.size();
	}
	return size;
}

double IndivActive::getPhi() const{
	unsigned phi = 0;
	for (auto & i : individuals_){
		phi += i.size();
	}
	return phi;
}

void IndivActive::shift(RNGcore * rng){
	if (individuals_.size() > 1){
		for (int i = individuals_.size()-2; i >= 0; --i){
			std::binomial_distribution Distr(individuals_[i].size(), rate_);
			unsigned n = Distr(*rng);
			Vec<unsigned> indices = sampleIndices(rng->get(), individuals_[i].size(), n);
			appendToEraseFromByIndices(individuals_[i+1], individuals_[i], indices);
		}
	}
}

IndivDiff IndivActive::getNewErased(RNGcore * rng) const{
	unsigned size = individuals_.back().size();
	std::binomial_distribution Distr(size, rate_);
	unsigned n = Distr(*rng);
	Vec<unsigned> indices = sampleIndices(rng->get(), size, n);
	return IndivDiff(patch_id_, indices);
}

IndivDiff IndivActive::sampleInfectors(RNGcore * rng, unsigned n) const{
	unsigned tot_indivs = 0;
	for (auto & i : individuals_){
		tot_indivs += i.size();
	}
	Vec<unsigned> indices = sampleIndicesWithReplacement(rng->get(), tot_indivs, n);
	std::sort(indices.begin(), indices.end());
	Vec<Individual> sampled(n);
	unsigned compartment = 0;
	unsigned prev_occupants = 0;
	for (unsigned i = 0; i < n; ++i){
		while (indices[i] >= prev_occupants + individuals_[compartment].size()){
			prev_occupants += individuals_[compartment].size();
			++compartment;
			/////////////////////////
			if (compartment == individuals_.size()){
				throw std::runtime_error("Error in sampleWithReplacement: too many advancements in compartments");
			}
			/////////////////////////
		}
		sampled[i] = individuals_[compartment][indices[i]-prev_occupants];
	}
	std::shuffle(sampled.begin(), sampled.end(), rng->get());
	return IndivDiff(patch_id_, sampled);
}


#endif