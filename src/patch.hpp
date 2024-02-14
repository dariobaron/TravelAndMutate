#ifndef PATCH_HPP
#define PATCH_HPP

#include <string>
#include "types.hpp"
#include "randomcore.hpp"
#include "recorder.hpp"
#include "pools.hpp"

class Patch{
	RNGcore * rng_;
	Recorder rec_;
	unsigned N_;
	double beta_;
	double epsilon_;
	double mu_;
	Pool * S_, * E_, * I_, * R_;
	Pool * Enew_, * Inew_, * Rnew_;
	bool uninitialized_;
public:
	Patch(RNGcore * rng, std::string pooltype, PatchProperties prop);
	~Patch();
	double getRho() const;
	const Recorder & getRecorder() const;
	Vec<unsigned> computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const;
	auto sampleInfectors(unsigned Enew) const;
	void addNewInfections(const Pool & Enew);
	void setNewRecoveries();
	void setNewOnsets();
	void update(Time t);
	bool isEpidemicAlive() const;
};


Patch::Patch(RNGcore * rng, std::string pooltype, PatchProperties prop) : uninitialized_(true){
	rng_ = rng;
	N_ = prop.N;
	beta_ = prop.beta;
	epsilon_ = prop.epsilon;
	mu_ = prop.mu;
	if (pooltype == "mix"){
		S_ = new Pool(N_-prop.I0); E_ = new Pool(prop.I0); I_ = new Pool(0); R_ = new Pool(0);
		Enew_ = new Pool(0); Inew_ = new Pool(0); Rnew_ = new Pool(0);
		uninitialized_ = false;
	}
	else if (pooltype == "individual"){
		S_ = new Reservoir(N_-prop.I0); E_ = new Individuals(prop.I0, N_); I_ = new Individuals(0, N_); R_ = new Pool(0);
		Enew_ = new Individuals(0, N_); Inew_ = new Individuals(0, N_); Rnew_ = new Individuals(0, N_);
		uninitialized_ = false;
	}
/*	else if (pooltype == "haplotype"){
		S_ = new Haplotype(N_); E_ = new Haplotype(0); I_ = new Haplotype(0); R_ = new Haplotype(0);
		Enew_ = new Haplotype(0); Inew_ = new Haplotype(0); Rnew_ = new Haplotype(0);
		uninitialized_ = false;
	}
*/	else {
		throw std::runtime_error("Pool type '"+pooltype+"' is not recognized");
	}
}


Patch::~Patch(){
	delete S_; delete E_; delete I_; delete R_;
	delete Enew_; delete Inew_; delete Rnew_;
}


double Patch::getRho() const{
	return I_->getPhi() / N_;
}


const Recorder & Patch::getRecorder() const{
	return rec_;
}


Vec<unsigned> Patch::computeInfections(const Vec<double> & rhos, const Vec<double> & c_ij) const{
	Vec<double> probs(rhos.size());
	std::transform(rhos.begin(), rhos.end(), c_ij.begin(), probs.begin(), std::multiplies<>());
	double f = beta_ * std::accumulate(probs.begin(), probs.end(), 0.);
	std::binomial_distribution Binom(S_->size(), f);
	unsigned Enew = Binom(*rng_);
	Vec<unsigned> Ninfectors(rhos.size(), 0);
	std::discrete_distribution Distr(probs.begin(), probs.end());
	for (unsigned i = 0; i < Enew; ++i){
		++Ninfectors[Distr(*rng_)];
	}
	return Ninfectors;
}


auto Patch::sampleInfectors(unsigned Ninfectors) const{
	return I_->sample(Ninfectors);
}


void Patch::addNewInfections(const Pool & Enew){
	*Enew_ += Enew;
}


void Patch::setNewRecoveries(){
	std::poisson_distribution Distr(mu_ * I_->size());
	unsigned Rnew = Distr(*rng_);
	Rnew = std::min(I_->size(), Rnew);
	*Rnew_ = I_->sample(Rnew);
}


void Patch::setNewOnsets(){
	std::poisson_distribution Distr(epsilon_ * E_->size());
	unsigned Inew = Distr(*rng_);
	Inew = std::min(E_->size(), Inew);
	*Inew_ = E_->sample(Inew);
}


void Patch::update(Time t){
	(*S_) -= (*Enew_);
	(*E_) += (*Enew_);
	(*E_) -= (*Inew_);
	(*I_) += (*Inew_);
	(*I_) -= (*Rnew_);
	(*R_) += (*Rnew_);
	rec_.push_trajectory(t, (*S_).size(), (*E_).size(), (*I_).size(), (*R_).size(), (*Enew_).size(), (*Inew_).size());
	Enew_->clear();
	Inew_->clear();
	Rnew_->clear();
}


bool Patch::isEpidemicAlive() const{
	return (*E_).size() + (*I_).size();
}

#endif
