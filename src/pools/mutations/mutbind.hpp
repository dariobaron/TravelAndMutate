#ifndef MUTBIND_HPP
#define MUTBIND_HPP

#include "../../algorithms.hpp"
#include "mutactive.hpp"
#include "mutpassive.hpp"
#include "mutdiff.hpp"

void MutDiff::moveFromTo(MutPassive & source, MutActive & target) const{
	target.hosts_.insert(target.hosts_.end(), hosts_.begin(), hosts_.end());
	source.size_ -= hosts_.size();
}

void MutDiff::moveFromTo(MutActive & source, MutActive & target) const{
	for (auto i : indices_){
		target.hosts_.push_back(std::move(source.hosts_[i]));
	}
	eraseWithoutOrder(source.hosts_, indices_);
}

void MutDiff::moveFromTo(MutActive & source, MutPassive & target) const{
	target.size_ += indices_.size();
	eraseWithoutOrder(source.hosts_, indices_);
}

#endif