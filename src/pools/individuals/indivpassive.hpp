#ifndef INDIVPASSIVE_HPP
#define INDIVPASSIVE_HPP

#include "../../types.hpp"
#include "../../individual.hpp"
#include "indivdiff.hpp"

class IndivPassive{
private:
	PatchID patch_id_;
	unsigned size_;
	unsigned count_;
public:
	friend class IndivDiff;
	IndivPassive(PatchID patch_id, unsigned n=0);
	unsigned size() const;
	IndivDiff generate(Time, unsigned n);
	IndivDiff generate(Time, const IndivDiff & other);
};

IndivPassive::IndivPassive(PatchID patch_id, unsigned n) : patch_id_(patch_id), size_(n), count_(0) {}

unsigned IndivPassive::size() const{
	return size_;
}

IndivDiff IndivPassive::generate(Time, unsigned n){
	Vec<Individual> v_ind(n);
	for (auto & i : v_ind){
		i = Individual(patch_id_, count_);
		++count_;
	}
	return IndivDiff(patch_id_, v_ind);
}

IndivDiff IndivPassive::generate(Time, const IndivDiff & other){
	Vec<Individual> v_ind(other.size());
	const Vec<Individual> & infectors = other.getIndividuals();
	for (unsigned i = 0; i < v_ind.size(); ++i){
		v_ind[i] = Individual(patch_id_, count_, infectors[i].patch_, infectors[i].id_);
		++count_;
	}
	return IndivDiff(patch_id_, v_ind);
}

#endif