#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <vector>
#include <string>
#include <exception>
#include <numeric>
#include "types.hpp"
#include "randomcore.hpp"
#include "patch.hpp"

class System{

	RNGcore * rng_;
	Vec<Patch> patches_;
	Vec<Vec<double>> c_ij_;
	Time t_;
	Time dt_;

public:

	System(RNGcore * rng, const np_array<double> & commuting_matrix, Time dt);

	template<typename ...Args>
	void setPatchProperties(unsigned i, Args... args);
	
	template<typename T>
	void seedEpidemic(Vec<T> I0s);

	void spreadForTime(Time tmax);

	auto getFullTrajectory(unsigned i);

private:

	bool isEpidemicAlive() const;

};

System::System(RNGcore * rng, const np_array<double> & commuting_matrix, Time dt) : rng_(rng), t_(0), dt_(dt){
	if (commuting_matrix.ndim() != 2){
		throw std::runtime_error("Commuting matrix must have 2 dimensions");
	}
	if (commuting_matrix.shape(0) != commuting_matrix.shape(1)){
		throw std::runtime_error("Commuting matrix must be squared");
	}
	unsigned nPatches = commuting_matrix.shape(0);
	auto view = commuting_matrix.unchecked<2>();
	patches_.resize(nPatches);
	for (unsigned i = 0; i < nPatches; ++i){
		c_ij_.emplace_back(commuting_matrix.data(i,0), view.data(i,nPatches));
	}
}

template<typename ...Args>
void System::setPatchProperties(unsigned i, Args... args){
	patches_[i].setProperties(rng_, args...);
}

template<typename T>
void System::seedEpidemic(Vec<T> I0s){
	if (I0s.size() != patches_.size()){
		throw std::runtime_error("Epidemic must be seeded with I0 array of length "+std::to_string(patches_.size()));
	}
	for (unsigned i = 0; i < I0s.size(); ++i){
		patches_[i].seed(I0s[i]);
	}
}

void System::spreadForTime(Time tmax){
	for (auto & p : patches_){
		p.update(t_);
	}
	Vec<double> rhos(patches_.size());
	while (t_ < tmax && isEpidemicAlive()){
		for (unsigned i = 0; i < patches_.size(); ++i){
			rhos[i] = patches_[i].getRho();
		}
		for (unsigned i = 0; i < patches_.size(); ++i){
			auto Ninfectors = patches_[i].computeInfections(rhos, c_ij_[i]);
			for (unsigned j = 0; j < Ninfectors.size(); ++j){
				auto infectors_ji = patches_[j].sampleInfectors(Ninfectors[j]);
				patches_[i].addNewInfections(infectors_ji);
			}
		}
		for (auto & p : patches_){
			p.setNewRecoveries();
			p.setNewOnsets();
		}
		t_ += dt_;
		for (auto & p : patches_){
			p.update(t_);
		}
	}
}

auto System::getFullTrajectory(unsigned i){
	return patches_[i].getRecorder().getFullTrajectory();
}

bool System::isEpidemicAlive() const{
	bool check = false;
	for (auto & p : patches_){
		check = check || p.isEpidemicAlive();
	}
	return check;
}

#endif