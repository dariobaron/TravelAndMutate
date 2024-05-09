#ifndef MUTBIND_HPP
#define MUTBIND_HPP

#include "../../algorithms.hpp"
#include "../../eventrecorder.hpp"
#include "mutactive.hpp"
#include "mutpassive.hpp"
#include "mutdiff.hpp"

void MutDiff::moveFromTo(Time t, MutPassive & source, MutActive & target) const{
	for (auto & h : hosts_){
		event_recorder.pushTransition(t, h.patch_, h.id_, 'E');
	}
	target.hosts_[0].insert(target.hosts_[0].end(), hosts_.begin(), hosts_.end());
	source.size_ -= hosts_.size();
}

void MutDiff::moveFromTo(Time t, MutActive & source, MutActive & target) const{
	for (auto i : indices_){
		auto h = source.hosts_.back()[i];
		event_recorder.pushTransition(t, h.patch_, h.id_, 'I');
	}
	appendToEraseFromByIndices(target.hosts_[0], source.hosts_.back(), indices_);
}

void MutDiff::moveFromTo(Time t, MutActive & source, MutPassive & target) const{
	for (auto i : indices_){
		auto h = source.hosts_.back()[i];
		event_recorder.pushTransition(t, h.patch_, h.id_, 'R');
	}
	target.size_ += indices_.size();
	for (auto i : indices_){
		mut_per_host_.push_back(source.hosts_.back()[i].nmut_);
	}
	eraseWithoutOrder(source.hosts_.back(), indices_);
}

#endif