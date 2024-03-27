#ifndef INDIVACTIVE_HPP
#define INDIVACTIVE_HPP

#include <algorithm>
#include <random>
#include "../../types.hpp"
#include "../../individual.hpp"
#include "../../randomcore.hpp"
#include "indivdiff.hpp"

class IndivActive{
private:
	PatchID patch_id_;
	Vec<Individual> individuals_;
public:
	friend class IndivDiff;
	IndivActive(PatchID patch_id);
	unsigned size() const;
	double getPhi() const;
	IndivDiff sample(RNGcore * rng, unsigned n) const;
	IndivDiff sampleWithReplacement(RNGcore * rng, unsigned n) const;
};

IndivActive::IndivActive(PatchID patch_id) : patch_id_(patch_id) {}

unsigned IndivActive::size() const{
	return individuals_.size();
}

double IndivActive::getPhi() const{
	return individuals_.size();
}

IndivDiff IndivActive::sample(RNGcore * rng, unsigned n) const{
	n = std::min(static_cast<unsigned>(individuals_.size()), n);
	Vec<unsigned> all_indices(individuals_.size());
	std::iota(all_indices.begin(), all_indices.end(), 0);
	Vec<unsigned> indices(n);
	std::sample(all_indices.begin(), all_indices.end(), indices.begin(), n, rng->get());
	return IndivDiff(patch_id_, indices);
}

IndivDiff IndivActive::sampleWithReplacement(RNGcore * rng, unsigned n) const{
	std::uniform_int_distribution<unsigned> Distr(0, individuals_.size()-1);
	Vec<Individual> sampled(n);
	for (auto & s : sampled){
		s = individuals_[Distr(rng->get())];
	}
	return IndivDiff(patch_id_, sampled);
}


#endif