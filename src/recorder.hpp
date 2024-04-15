#ifndef RECORDER_HPP
#define RECORDER_HPP

#include <vector>
#include "types.hpp"

class Recorder{

public:
	Vec<FullTraj> dyn_;
	Vec<InfecTree> tree_;
	Vec<MutTree> mutations_;
public:
	template<typename ...Args>
	void push_trajectory(Args ... args);
	np_array<FullTraj> getFullTrajectory() const;
	template<typename ...Args>
	void push_tree(Args ... args);
	np_array<InfecTree> getInfectionTree() const;
	template<typename ...Args>
	void push_host(Args ... args);
	np_array<MutTree> getMutationTree() const;
};

template<typename ...Args>
void Recorder::push_trajectory(Args ... args){
	dyn_.emplace_back(args...);
}

np_array<FullTraj> Recorder::getFullTrajectory() const{
	np_array<FullTraj> records(dyn_.size());
	auto view = records.mutable_unchecked<1>();
	for (unsigned i = 0; i < dyn_.size(); ++i){
		view(i) = dyn_[i];
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