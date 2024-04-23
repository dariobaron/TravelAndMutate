#ifndef INDIVBIND_HPP
#define INDIVBIND_HPP

#include "../../algorithms.hpp"
#include "indivactive.hpp"
#include "indivpassive.hpp"
#include "indivdiff.hpp"

void IndivDiff::moveFromTo(Time, IndivPassive & source, IndivActive & target) const{
	target.individuals_[0].insert(target.individuals_[0].end(), individuals_.begin(), individuals_.end());
	source.size_ -= individuals_.size();
}

void IndivDiff::moveFromTo(Time, IndivActive & source, IndivActive & target) const{
	appendToEraseFromByIndices(target.individuals_[0], source.individuals_.back(), indices_);
}

void IndivDiff::moveFromTo(Time, IndivActive & source, IndivPassive & target) const{
	target.size_ += indices_.size();
	eraseWithoutOrder(source.individuals_.back(), indices_);
}

#endif