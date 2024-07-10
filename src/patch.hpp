#ifndef PATCH_HPP
#define PATCH_HPP

#include <type_traits>
#include "types.hpp"
#include "randomcore.hpp"
#include "haplotypes.hpp"
#include "sequencer.hpp"
#include "recorder.hpp"
#include "pools.hpp"

template<Pool PoolType>
class Patch{
	RNGcore * rng_;
	Sequencer * sequencer_;
	Recorder<PoolType> * rec_;
	unsigned N_;
	double beta_;
	double epsilon_;
	double mu_;
	unsigned I0_;
	PatchID patch_id_;
	PoolType::Passive S_, R_;
	PoolType::Active E_, I_;
	PoolType::Diff Enew_, Inew_, Rnew_;
public:
	Patch(RNGcore * rng, PatchID patch_id, unsigned gamma_trick, PatchProperties prop);
	double getRho() const;
	void setRecorder(Recorder<PoolType> * recorder);
	void setHaplotypes(Haplotypes * seqdealer);
	void setSequencer(Sequencer * sequencer);
	void seedEpidemic();
	Vec<unsigned> computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const;
	auto sampleInfectors(unsigned Enew) const;
	void addNewInfections(Time t, const PoolType::Diff & Enew);
	void setNewRecoveries();
	void setNewOnsets();
	void sampleSequences();
	void update(Time t);
	bool isEpidemicAlive() const;
};


template<Pool PoolType>
Patch<PoolType>::Patch(RNGcore * rng, PatchID patch_id, unsigned gamma_trick, PatchProperties prop) : 
		rng_(rng), N_(prop.N), beta_(prop.beta), epsilon_(gamma_trick*prop.epsilon), mu_(gamma_trick*prop.mu), I0_(prop.I0), patch_id_(patch_id),
		S_(patch_id,N_), R_(patch_id),
		E_(patch_id, gamma_trick, epsilon_), I_(patch_id, gamma_trick, mu_),
		Enew_(patch_id), Inew_(patch_id), Rnew_(patch_id){}


template<Pool PoolType>
double Patch<PoolType>::getRho() const{
	return I_.getPhi() / N_;
}


template<Pool PoolType>
void Patch<PoolType>::setRecorder(Recorder<PoolType> * recorder){
	rec_ = recorder;
}


template<Pool PoolType>
void Patch<PoolType>::setHaplotypes(Haplotypes * haplos){
	S_.setHaplotypes(haplos);
	E_.setHaplotypes(haplos);
	I_.setHaplotypes(haplos);
}


template<Pool PoolType>
void Patch<PoolType>::setSequencer(Sequencer * sequencer){
	sequencer_ = sequencer;
	I_.setSequencer(sequencer);
	Inew_.setSequencer(sequencer);
	Rnew_.setSequencer(sequencer);
}


template<Pool PoolType>
void Patch<PoolType>::seedEpidemic(){
	Enew_ = S_.generate(0, I0_);
	if constexpr (std::is_same<PoolType,Individuals>::value){
		for (auto & i : Enew_.getIndividuals()){
			rec_->pushInfection(Time(0), i.patch_, i.id_, i.infector_patch_, i.infector_id_);
		}
	}
	if constexpr (std::is_same<PoolType,Mutations>::value){
		for (auto & i : Enew_.getHosts()){
			rec_->pushInfection(Time(0), i.patch_, i.id_, i.mut_, i.infector_patch_, i.infector_id_, i.infector_mut_);
		}
	}
	Enew_.moveFromTo(S_, E_);
	Enew_.clear();
}


template<Pool PoolType>
Vec<unsigned> Patch<PoolType>::computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const{
	Vec<double> probs(rhos.size());
	std::transform(rhos.begin(), rhos.end(), c_ij.begin(), probs.begin(), std::multiplies<>());
	double f = beta_ * std::accumulate(probs.begin(), probs.end(), 0.);
	std::binomial_distribution Binom(S_.size(), f);
	unsigned Enew = Binom(*rng_);
	Vec<unsigned> Ninfectors(rhos.size(), 0);
	std::discrete_distribution Distr(probs.begin(), probs.end());
	for (unsigned i = 0; i < Enew; ++i){
		++Ninfectors[Distr(*rng_)];
	}
	return Ninfectors;
}


template<Pool PoolType>
auto Patch<PoolType>::sampleInfectors(unsigned Ninfectors) const{
	return I_.sampleInfectors(rng_, Ninfectors);
}


template<Pool PoolType>
void Patch<PoolType>::addNewInfections(Time t, const PoolType::Diff & Enew){
	Enew_ += S_.generate(t, Enew);
}


template<Pool PoolType>
void Patch<PoolType>::setNewRecoveries(){
	Rnew_ = I_.getNewErased(rng_);
	if constexpr (std::is_same<PoolType,Mutations>::value){
		Rnew_.setSequencer(sequencer_);
	}
}


template<Pool PoolType>
void Patch<PoolType>::setNewOnsets(){
	Inew_ = E_.getNewErased(rng_);
	if constexpr (std::is_same<PoolType,Mutations>::value){
		Inew_.setSequencer(sequencer_);
	}
}


template<Pool PoolType>
void Patch<PoolType>::sampleSequences(){
	I_.sampleSequences();
}


template<Pool PoolType>
void Patch<PoolType>::update(Time t){
	Rnew_.moveFromTo(I_, R_);
	I_.shift(rng_);
	Inew_.moveFromTo(E_, I_);
	E_.shift(rng_);
	Enew_.moveFromTo(S_, E_);
	rec_->pushTrajectory(patch_id_, t, S_.size(), E_.size(), I_.size(), R_.size(), Enew_.size(), Inew_.size());
	if constexpr (std::is_same<PoolType,Individuals>::value){
		for (auto & i : Enew_.getIndividuals()){
			rec_->pushInfection(t, i.patch_, i.id_, i.infector_patch_, i.infector_id_);
		}
	}
	if constexpr (std::is_same<PoolType,Mutations>::value){
		E_.updateHaplotypes(t);
		I_.updateHaplotypes(t);
		for (auto & i : Enew_.getHosts()){
			rec_->pushInfection(t, i.patch_, i.id_, i.mut_, i.infector_patch_, i.infector_id_, i.infector_mut_);
		}
	}
	Enew_.clear();
	Inew_.clear();
	Rnew_.clear();
}


template<Pool PoolType>
bool Patch<PoolType>::isEpidemicAlive() const{
	return E_.size() + I_.size();
}

#endif
