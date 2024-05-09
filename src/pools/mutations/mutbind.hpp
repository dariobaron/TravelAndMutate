#ifndef MUTBIND_HPP
#define MUTBIND_HPP

#include "../../algorithms.hpp"
#include "mutactive.hpp"
#include "mutpassive.hpp"
#include "mutdiff.hpp"

void MutDiff::moveFromTo(MutPassive & source, MutActive & target) const{
	target.hosts_[0].insert(target.hosts_[0].end(), hosts_.begin(), hosts_.end());
	source.size_ -= hosts_.size();
}

void MutDiff::moveFromTo(MutActive & source, MutActive & target) const{
	appendToEraseFromByIndices(target.hosts_[0], source.hosts_.back(), indices_);
}

void MutDiff::moveFromTo(MutActive & source, MutPassive & target) const{
	target.size_ += indices_.size();
	for (auto i : indices_){
		mut_per_host_.push_back(source.hosts_.back()[i].nmut_);
	}
	eraseWithoutOrder(source.hosts_.back(), indices_);
}

#endif