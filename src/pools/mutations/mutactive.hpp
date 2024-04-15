#ifndef MUTACTIVE_HPP
#define MUTACTIVE_HPP

#include <algorithm>
#include <random>
#include "../../types.hpp"
#include "../../host.hpp"
#include "../../randomcore.hpp"
#include "mutdiff.hpp"

class MutActive{
private:
	PatchID patch_id_;
	Vec<Host> hosts_;
public:
	friend class MutDiff;
	MutActive(PatchID patch_id);
	unsigned size() const;
	double getPhi() const;
	Vec<Host>& getHosts();
	MutDiff sample(RNGcore * rng, unsigned n) const;
	MutDiff sampleWithReplacement(RNGcore * rng, unsigned n) const;
};

MutActive::MutActive(PatchID patch_id) : patch_id_(patch_id) {}

unsigned MutActive::size() const{
	return hosts_.size();
}

double MutActive::getPhi() const{
	return hosts_.size();
}

Vec<Host>& MutActive::getHosts(){
	return hosts_;
}

MutDiff MutActive::sample(RNGcore * rng, unsigned n) const{
	n = std::min(static_cast<unsigned>(hosts_.size()), n);
	Vec<unsigned> all_indices(hosts_.size());
	std::iota(all_indices.begin(), all_indices.end(), 0);
	Vec<unsigned> indices(n);
	std::sample(all_indices.begin(), all_indices.end(), indices.begin(), n, rng->get());
	return MutDiff(patch_id_, indices);
}

MutDiff MutActive::sampleWithReplacement(RNGcore * rng, unsigned n) const{
	std::uniform_int_distribution<unsigned> Distr(0, hosts_.size()-1);
	Vec<Host> sampled(n);
	for (auto & s : sampled){
		s = hosts_[Distr(rng->get())];
	}
	return MutDiff(patch_id_, sampled);
}


#endif