#ifndef MUTBIND_HPP
#define MUTBIND_HPP

#include "mixlightlib/algorithms.hpp"
#include "mutactive.hpp"
#include "mutpassive.hpp"
#include "mutdiff.hpp"

void MutDiff::moveFromTo(MutPassive & source, MutActive & target) const{
	target.hosts_[0].insert(target.hosts_[0].end(), hosts_.begin(), hosts_.end());
	source.size_ -= hosts_.size();
}

void MutDiff::moveFromTo(MutActive & source, MutActive & target) const{
	appendToEraseFromByIndices(target.hosts_[0], source.hosts_.back(), indices_);
	for (auto it = target.hosts_[0].rbegin(); it != target.hosts_[0].rbegin()+indices_.size(); ++it){
		if (sequencer_->toBeSequenced()){
			it->t_sequencing_ = sequencer_->extractSamplingTime();
		}
	}
}

void MutDiff::moveFromTo(MutActive & source, MutPassive & target) const{
	target.size_ += indices_.size();
	const auto & healing_hosts = source.hosts_.back();
	for (auto i : indices_){
		sequencer_->recordOnHealing(healing_hosts[i]);
	}
	eraseWithoutOrder(source.hosts_.back(), indices_);
}

#endif