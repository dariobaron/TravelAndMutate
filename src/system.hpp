#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <vector>
#include <string>
#include <exception>
#include <numeric>
#include "types.hpp"
#include "randomcore.hpp"
#include "haplotypes.hpp"
#include "sequencer.hpp"
#include "patch.hpp"
#include "trees.hpp"

template<Pool PoolType>
class System{
	RNGcore * rng_;
	Haplotypes * haplos_;
	Sequencer * sequencer_;
	Vec<Patch<PoolType>> patches_;
	Vec<Vec<double>> c_ij_;
	Time t_;
	bool seeded_;
	bool verbose_;
public:
	System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties, unsigned gamma_trick);
	void setRecorder(Recorder<PoolType> * recorder);
	void setHaplotypes(Haplotypes * seqdealer);
	void setSequencer(Sequencer * sequencer);
	void setVerbosity(bool verbose);
	void seedEpidemic();
	void spreadForTime(Time tmax);
private:
	bool isEpidemicAlive() const;
	void checkHaploDealer() const;
	void checkSequencer() const;
	void checkSeeded() const;
	void updatePatches();
};


template<Pool PoolType>
System<PoolType>::System(RNGcore * rng, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties, unsigned gamma_trick) :
				rng_(rng), haplos_(nullptr), sequencer_(nullptr), t_(0), seeded_(false), verbose_(false) {
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
void System<PoolType>::setRecorder(Recorder<PoolType> * recorder){
	for (auto & p : patches_){
		p.setRecorder(recorder);
	}
}


template<Pool PoolType>
void System<PoolType>::setHaplotypes(Haplotypes * haplos){
	haplos_ = haplos;
	for (auto & p : patches_){
		p.setHaplotypes(haplos_);
	}
}


template<Pool PoolType>
void System<PoolType>::setSequencer(Sequencer * sequencer){
	sequencer_ = sequencer;
	for (auto & p : patches_){
		p.setSequencer(sequencer_);
	}
}


template<Pool PoolType>
void System<PoolType>::setVerbosity(bool verbose){
	verbose_ = verbose;
}


template<Pool PoolType>
void System<PoolType>::seedEpidemic(){
	checkHaploDealer();
	checkSequencer();
	seeded_ = true;
	for (auto & p : patches_){
		p.seedEpidemic();
	}
	sequencer_->update(Time(0));
	Host fictious_host;
	fictious_host.t_sequencing_ = 0;
	fictious_host.patch_ = -1;
	fictious_host.evolved_mut_ = 0;
	sequencer_->record(fictious_host);
}


template<Pool PoolType>
void System<PoolType>::spreadForTime(Time tmax){
	checkSeeded();
	checkHaploDealer();
	checkSequencer();
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
		if constexpr (std::is_same<PoolType,Mutations>::value){
			for (auto & p : patches_){
				p.sampleSequences();
			}
		}
		++t_;
		sequencer_->update(t_);
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
void System<PoolType>::checkSequencer() const{
	if constexpr (std::is_same<PoolType,Mutations>::value){
		if (!static_cast<bool>(sequencer_)){
			throw std::runtime_error("Spreading required before setting Sequencer");
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