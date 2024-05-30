#ifndef RECORDER_HPP
#define RECORDER_HPP

#include <vector>
#include "types.hpp"
#include "trees.hpp"

class Recorder{
	Vec<Vec<FullTraj>> dyn_;
	Vec<InfecTree> tree_;
	Vec<MutTree> mutations_;
public:
	Recorder(unsigned Npatches);
	template<typename ...Args>
	void push_trajectory(PatchID patch, Args ... args);
	np_array<FullTraj> getFullTrajectory(PatchID patch) const;
	template<typename ...Args>
	void push_tree(Args ... args);
	np_array<InfecTree> getInfectionTree() const;
	auto getTreeBalance() const;
	template<typename ...Args>
	void push_host(Args ... args);
	np_array<MutTree> getMutationTree() const;
};

Recorder::Recorder(unsigned Npatches) : dyn_(Npatches) {}

template<typename ...Args>
void Recorder::push_trajectory(PatchID patch, Args ... args){
	dyn_[patch].emplace_back(args...);
}

np_array<FullTraj> Recorder::getFullTrajectory(PatchID patch) const{
	const Vec<FullTraj> & dynamics = dyn_[patch];
	np_array<FullTraj> records(dynamics.size());
	auto view = records.mutable_unchecked<1>();
	for (unsigned i = 0; i < dynamics.size(); ++i){
		view(i) = dynamics[i];
	}
	return records;
}

template<typename ...Args>
void Recorder::push_tree(Args ... args){
	tree_.emplace_back(args...);
}

np_array<InfecTree> Recorder::getInfectionTree() const{
	np_array<InfecTree> records(tree_.size());
	auto view = records.mutable_unchecked<1>();
	for (unsigned i = 0; i < tree_.size(); ++i){
		view(i) = tree_[i];
	}
	return records;
}

auto Recorder::getTreeBalance() const{
	return treebalanceTree(tree_);
}

template<typename ...Args>
void Recorder::push_host(Args ... args){
	mutations_.emplace_back(args...);
}

np_array<MutTree> Recorder::getMutationTree() const{
	np_array<MutTree> records(mutations_.size());
	auto view = records.mutable_unchecked<1>();
	for (unsigned i = 0; i < mutations_.size(); ++i){
		view(i) = mutations_[i];
	}
	return records;
}


#endif