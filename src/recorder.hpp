#ifndef RECORDER_HPP
#define RECORDER_HPP

#include <vector>
#include "pools.hpp"
#include "types.hpp"
#include "trees.hpp"

template<Pool PoolType>
class Recorder{
public:
	using InfectRecord = PoolType::InfectRec;
private:
	Vec<Vec<FullTraj>> dyn_;
	Vec<InfectRecord> infections_;
public:
	Recorder(unsigned Npatches);
	template<typename ...Args>
	void pushTrajectory(PatchID patch, Args ... args);
	auto getFullTrajectory(PatchID patch) const;
	template<typename ...Args>
	void pushInfection(Args ... args);
	auto getInfectionTree() const;
	auto getTreeBalance() const;
};

template<Pool PoolType>
Recorder<PoolType>::Recorder(unsigned Npatches) : dyn_(Npatches) {}

template<Pool PoolType>
template<typename ...Args>
void Recorder<PoolType>::pushTrajectory(PatchID patch, Args ... args){
	dyn_[patch].emplace_back(args...);
}

template<Pool PoolType>
auto Recorder<PoolType>::getFullTrajectory(PatchID patch) const{
	const Vec<FullTraj> & dynamics = dyn_[patch];
	np_array<FullTraj> records(dynamics.size());
	auto view = records.mutable_unchecked<1>();
	for (unsigned i = 0; i < dynamics.size(); ++i){
		view(i) = dynamics[i];
	}
	return records;
}

template<Pool PoolType>
template<typename ...Args>
void Recorder<PoolType>::pushInfection(Args ... args){
	infections_.emplace_back(args...);
}
template<>
template<typename ...Args>
void Recorder<Mix>::pushInfection(Args ... args){}

template<Pool PoolType>
auto Recorder<PoolType>::getInfectionTree() const{
	np_array<InfectRecord> records(infections_.size());
	auto view = records.template mutable_unchecked<1>();
	for (unsigned i = 0; i < infections_.size(); ++i){
		view(i) = infections_[i];
	}
	return records;
}
template<>
auto Recorder<Mix>::getInfectionTree() const{}

template<Pool PoolType>
auto Recorder<PoolType>::getTreeBalance() const{
	return treebalanceTree(infections_);
}


#endif