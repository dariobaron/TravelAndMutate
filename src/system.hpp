#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <vector>
#include <string>
#include <exception>
#include <numeric>
#include "types.hpp"
#include "randomcore.hpp"
#include "haplotypes.hpp"
#include "patch.hpp"
#include "trees.hpp"
#include "eventrecorder.hpp"

template<Pool PoolType>
class System{
	RNGcore * rng_;
	Haplotypes * haplos_;
	Vec<Patch<PoolType>> patches_;
	Vec<Vec<double>> c_ij_;
	Time t_;
	bool seeded_;
	bool verbose_;
public:
	System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties, unsigned gamma_trick);
	void setVerbosity(bool verbose);
	void seedEpidemic();
	void spreadForTime(Time tmax);
	auto getFullTrajectory(unsigned i) const;
	auto getInfectionTree(unsigned i) const;
	auto getTreeBalance() const;
	auto getMutationTree(unsigned i) const;
	auto getEventRecorder() const;
	void setHaplotypes(Haplotypes * seqdealer);
private:
	bool isEpidemicAlive() const;
	void checkHaploDealer() const;
	void checkSeeded() const;
	void updatePatches();
};


template<Pool PoolType>
System<PoolType>::System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties, unsigned gamma_trick) :
				rng_(rng), haplos_(nullptr), t_(0), seeded_(false), verbose_(false) {
	if (commuting_matrix.ndim() != 2){
		throw std::runtime_error("Commuting matrix must have 2 dimensions");
	}
	if (commuting_matrix.shape(0) != commuting_matrix.shape(1)){
		throw std::runtime_error("Commuting matrix must be squared");
	}
	if (commuting_matrix.shape(0) != properties.shape(0)){
		throw std::runtime_error("Shape of commuting matrix and properties must match");
	}
	unsigned nPatches = commuting_matrix.shape(0);
	auto view = commuting_matrix.unchecked<2>();
	for (unsigned i = 0; i < nPatches; ++i){
		c_ij_.emplace_back(view.data(i,0), view.data(i,nPatches));
		patches_.emplace_back(rng_, i, gamma_trick, properties.at(i));
	}
}


template<Pool PoolType>
void System<PoolType>::setVerbosity(bool verbose){
	verbose_ = verbose;
}


template<Pool PoolType>
void System<PoolType>::seedEpidemic(){
	checkHaploDealer();
	seeded_ = true;
	for (auto & p : patches_){
		p.seedEpidemic();
	}
}


template<Pool PoolType>
void System<PoolType>::spreadForTime(Time tmax){
	checkSeeded();
	checkHaploDealer();
	updatePatches();
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
		updatePatches();
		if (verbose_){
			py::print("Simulation at step", t_, py::arg("end")="\r", py::arg("flush")=true);
		}
	}
	if (verbose_){
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
auto System<PoolType>::getEventRecorder() const{
	return event_recorder;
}


template<Pool PoolType>
void System<PoolType>::setHaplotypes(Haplotypes * haplos){
	haplos_ = haplos;
	for (auto & p : patches_){
		p.setHaplotypes(haplos_);
	}
}


template<Pool PoolType>
bool System<PoolType>::isEpidemicAlive() const{
	bool check = false;
	for (auto & p : patches_){
		check = check || p.isEpidemicAlive();
	}
	return check;
}


template<Pool PoolType>
void System<PoolType>::checkHaploDealer() const{
	if constexpr (std::is_same<PoolType,Mutations>::value){
		if (!static_cast<bool>(haplos_)){
			throw std::runtime_error("Spreading required before setting Haplotypes");
		}
	}
}


template<Pool PoolType>
void System<PoolType>::checkSeeded() const{
	if (!seeded_){
		throw std::runtime_error("Spreading required before seeding");
	}
}


template<Pool PoolType>
void System<PoolType>::updatePatches(){
	for (auto & p : patches_){
		p.update(t_);
	}
}

#endif