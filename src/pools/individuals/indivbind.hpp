#ifndef INDIVBIND_HPP
#define INDIVBIND_HPP

#include "../../algorithms.hpp"
#include "indivactive.hpp"
#include "indivpassive.hpp"
#include "indivdiff.hpp"

void IndivDiff::moveFromTo(IndivPassive & source, IndivActive & target) const{
	target.individuals_.insert(target.individuals_.end(), individuals_.begin(), individuals_.end());
	source.size_ -= individuals_.size();
}

void IndivDiff::moveFromTo(IndivActive & source, IndivActive & target) const{
	for (auto i : indices_){
		target.individuals_.push_back(std::move(source.individuals_[i]));
	}
	eraseWithoutOrder(source.individuals_, indices_);
}

void IndivDiff::moveFromTo(IndivActive & source, IndivPassive & target) const{
	target.size_ += indices_.size();
	eraseWithoutOrder(source.individuals_, indices_);
}

#endif