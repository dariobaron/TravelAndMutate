#ifndef PATCH_HPP
#define PATCH_HPP

#include <type_traits>
#include "types.hpp"
#include "randomcore.hpp"
#include "recorder.hpp"
#include "pools.hpp"

template<Pool PoolType>
class Patch{
	RNGcore * rng_;
	Recorder rec_;
	unsigned N_;
	double beta_;
	double epsilon_;
	double mu_;
	PatchID patch_id_;
	PoolType::Passive S_, R_;
	PoolType::Active E_, I_;
	PoolType::Diff Enew_, Inew_, Rnew_;
public:
	Patch(RNGcore * rng, PatchID patch_id, PatchProperties prop);
	double getRho() const;
	const Recorder & getRecorder() const;
	Vec<unsigned> computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const;
	auto sampleInfectors(unsigned Enew) const;
	void addNewInfections(Time t, const PoolType::Diff & Enew);
	void setNewRecoveries();
	void setNewOnsets();
	void update(Time t);
	bool isEpidemicAlive() const;
};


template<Pool PoolType>
Patch<PoolType>::Patch(RNGcore * rng, PatchID patch_id, PatchProperties prop) : 
		rng_(rng), N_(prop.N), beta_(prop.beta), epsilon_(prop.epsilon), mu_(prop.mu), patch_id_(patch_id),
		S_(patch_id,N_), R_(patch_id),
		E_(patch_id), I_(patch_id),
		Enew_(patch_id), Inew_(patch_id), Rnew_(patch_id)
{
	Enew_ = S_.generate(0, prop.I0);
	Enew_.moveFromTo(S_, E_);
	if constexpr (std::is_same<PoolType,Individuals>::value){
		for (auto & i : Enew_.getIndividuals()){
			rec_.push_tree(Time(0), i.patch_, i.id_, i.infector_patch_, i.infector_id_);
		}
	}
	Enew_.clear();
}


template<Pool PoolType>
double Patch<PoolType>::getRho() const{
	return I_.getPhi() / N_;
}


template<Pool PoolType>
const Recorder & Patch<PoolType>::getRecorder() const{
	return rec_;
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
	return I_.sampleWithReplacement(rng_, Ninfectors);
}


template<Pool PoolType>
void Patch<PoolType>::addNewInfections(Time t, const PoolType::Diff & Enew){
	Enew_ += S_.generate(t, Enew);
}


template<Pool PoolType>
void Patch<PoolType>::setNewRecoveries(){
	std::binomial_distribution Distr(I_.size(), mu_);
	unsigned Rnew = Distr(*rng_);
	Rnew_ = I_.sample(rng_, Rnew);
}


template<Pool PoolType>
void Patch<PoolType>::setNewOnsets(){
	std::binomial_distribution Distr(E_.size(), epsilon_);
	unsigned Inew = Distr(*rng_);
	Inew_ = E_.sample(rng_, Inew);
}


template<Pool PoolType>
void Patch<PoolType>::update(Time t){
	Rnew_.moveFromTo(I_, R_);
	Inew_.moveFromTo(E_, I_);
	Enew_.moveFromTo(S_, E_);
	rec_.push_trajectory(t, S_.size(), E_.size(), I_.size(), R_.size(), Enew_.size(), Inew_.size());
	if constexpr (std::is_same<PoolType,Individuals>::value){
		for (auto & i : Enew_.getIndividuals()){
			rec_.push_tree(t, i.patch_, i.id_, i.infector_patch_, i.infector_id_);
		}
	}
	if constexpr (std::is_same<PoolType,Mutations>::value){
		for (auto & i : I_.getHosts()){
			if (t >= i.t_next_mut_){
				Time tnext = t + PoolType::Passive::allmutations.nextMutation();
				i.evolveMutation(t, PoolType::Passive::allmutations.newMutation(), tnext);
			}
		}
		for (auto & i : Enew_.getHosts()){
			rec_.push_host(t, i.patch_, i.id_, i.mut_, i.infector_patch_, i.infector_id_, i.infector_mut_);
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
