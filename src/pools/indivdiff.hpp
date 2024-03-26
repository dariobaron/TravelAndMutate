#ifndef INDIVDIFF_HPP
#define INDIVDIFF_HPP

#include "../types.hpp"
#include "../individual.hpp"

class IndivPassive;
class IndivActive;

class IndivDiff{
private:
	PatchID patch_id_;
	Vec<Individual> individuals_;
	Vec<unsigned> indices_;
public:
	IndivDiff(PatchID patch_id);
	IndivDiff(PatchID patch_id, Vec<Individual> v);
	IndivDiff(PatchID patch_id, Vec<unsigned> v);
	unsigned size() const;
	const Vec<Individual>& getIndividuals() const;
	IndivDiff& operator+=(const IndivDiff & other);
	void clear();
	void moveFromTo(IndivPassive & source, IndivActive & target) const;
	void moveFromTo(IndivActive & source, IndivActive & target) const;
	void moveFromTo(IndivActive & source, IndivPassive & target) const;
};

IndivDiff::IndivDiff(PatchID patch_id) : patch_id_(patch_id) {}

IndivDiff::IndivDiff(PatchID patch_id, Vec<Individual> v) : patch_id_(patch_id), individuals_(v) {}

IndivDiff::IndivDiff(PatchID patch_id, Vec<unsigned> v) : patch_id_(patch_id), indices_(v) {}

unsigned IndivDiff::size() const{
	return std::max(individuals_.size(), indices_.size());
}

const Vec<Individual>& IndivDiff::getIndividuals() const{
	return individuals_;
}

IndivDiff& IndivDiff::operator+=(const IndivDiff & other){
	individuals_.insert(individuals_.end(), other.individuals_.begin(), other.individuals_.end());
	indices_.insert(indices_.end(), other.indices_.begin(), other.indices_.end());
	return *this;
}

void IndivDiff::clear(){
	individuals_.clear();
	indices_.clear();
}


#endif