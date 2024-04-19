#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <vector>
#include <string>
#include <exception>
#include <numeric>
#include "types.hpp"
#include "randomcore.hpp"
#include "patch.hpp"
#include "trees.hpp"

template<Pool PoolType>
class System{
	RNGcore * rng_;
	Vec<Patch<PoolType>> patches_;
	Vec<Vec<double>> c_ij_;
	Time t_;
	bool verbose;
public:
	System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties);
	void setVerbosity();
	void seedEpidemic();
	void spreadForTime(Time tmax);
	auto getFullTrajectory(unsigned i) const;
	auto getInfectionTree(unsigned i) const;
	auto getTreeBalance() const;
	auto getMutationTree(unsigned i) const;
	void setMutationRate(double mut_rate);
private:
	bool isEpidemicAlive() const;
};


template<Pool PoolType>
System<PoolType>::System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties) :
				rng_(rng), t_(0), verbose(false) {
	if (commuting_matrix.ndim() != 2){
		throw std::runtime_error("Commuting matrix must have 2 dimensions");
	}
	if (commuting_matrix.shape(0) != commuting_matrix.shape(1)){
		throw std::runtime_error("Commuting matrix must be squared");
	}
	if (commuting_matrix.shape(0) != properties.shape(0)){
		throw std::runtime_error("Shape of commuting matrix and properties must match");
	}
	if constexpr (std::is_same<PoolType,Mutations>::value){
		PoolType::Passive::allmutations.setRNG(rng_);
	}
	unsigned nPatches = commuting_matrix.shape(0);
	auto view = commuting_matrix.unchecked<2>();
	for (unsigned i = 0; i < nPatches; ++i){
		c_ij_.emplace_back(view.data(i,0), view.data(i,nPatches));
		patches_.emplace_back(rng_, i, properties.at(i));
	}
}


template<Pool PoolType>
void System<PoolType>::setVerbosity(){
	verbose = true;
}


template<Pool PoolType>
void System<PoolType>::seedEpidemic(){
	for (auto & p : patches_){
		p.seedEpidemic();
	}
}


template<Pool PoolType>
void System<PoolType>::spreadForTime(Time tmax){
	for (auto & p : patches_){
		p.update(t_);
	}
	Vec<double> rhos(patches_.size());
	while (t_ < tmax && isEpidemicAlive()){
		for (unsigned i = 0; i < patches_.size(); ++i){
			rhos[i] = patches_[i].getRho();
		}
		for (unsigned i = 0; i < patches_.size(); ++i){
			Vec<unsigned> Ninfectors = patches_[i].computeInfections(rhos, c_ij_[i]);
			for (unsigned j = 0; j < Ninfectors.size(); ++j){
				auto infectors_ji = patches_[j].sampleInfectors(Ninfectors[j]);
				patches_[i].addNewInfections(t_, infectors_ji);
			}
		}
		for (auto & p : patches_){
			p.setNewRecoveries();
			p.setNewOnsets();
		}
		++t_;
		for (auto & p : patches_){
			p.update(t_);
		}
		if (verbose){
			py::print("Simulation at step", t_, py::arg("end")="\r", py::arg("flush")=true);
		}
	}
	if (verbose){
		py::print("Simulation finished in", t_, "steps", py::arg("flush")=true);
	}
}


template<Pool PoolType>
auto System<PoolType>::getFullTrajectory(unsigned i) const{
	return patches_[i].getRecorder().getFullTrajectory();
}


template<Pool PoolType>
auto System<PoolType>::getInfectionTree(unsigned i) const{
	return patches_[i].getRecorder().getInfectionTree();
}


template<Pool PoolType>
auto System<PoolType>::getTreeBalance() const{
	Vec<const Vec<InfecTree> *> trees;
	for (auto & p : patches_){
		trees.push_back(&p.getRecorder().tree_);
	}
	return treebalanceTree(trees);
}


template<Pool PoolType>
auto System<PoolType>::getMutationTree(unsigned i) const{
	return patches_[i].getRecorder().getMutationTree();
}


template<Pool PoolType>
void System<PoolType>::setMutationRate(double mut_rate){
	PoolType::Passive::allmutations.setMutationRate(mut_rate);
}


template<Pool PoolType>
bool System<PoolType>::isEpidemicAlive() const{
	bool check = false;
	for (auto & p : patches_){
		check = check || p.isEpidemicAlive();
	}
	return check;
}

#endif