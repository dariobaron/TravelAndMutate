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

	System(RNGcore * rng, Time dt, std::string pooltype, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties);

	void spreadForTime(Time tmax);

	auto getFullTrajectory(unsigned i);

private:

	bool isEpidemicAlive() const;

};

System::System(RNGcore * rng, Time dt, std::string pooltype, const np_array<double> & commuting_matrix, const np_array<PatchProperties> & properties) :
				rng_(rng), t_(0), dt_(dt){
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
		patches_.emplace_back(rng_, pooltype, properties.at(i));
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