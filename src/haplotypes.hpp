#ifndef HAPLOTYPES_HPP
#define HAPLOTYPES_HPP

#include <map>
#include <vector>
#include <string>
#include <random>
#include <pybind11/stl.h>
#include "types.hpp"
#include "randomcore.hpp"
#include "sequence.hpp"
#include "../mixlightlib/include/random.hpp"


class Haplotypes{
private:
	RNGcore * rng_;
	double mr_;
	Vec<Sequence<seq_len>> seqs_;
	Vec<unsigned> parents_;
	Vec<Time> birth_ts_;
	Vec<PatchID> birth_locs_;
	Vec<double> phi_h_;
	std::gamma_distribution<> mut_period_;
	DiscreteDistribution<double> phi_shifter_;
public:
	Haplotypes(RNGcore * rng, std::map<std::string,double> properties);
	Haplotypes(const np_array<MutationTree> & muttree);
	double getMutationRate() const;
	double getPhiH(unsigned i) const;
	unsigned getTotal() const;
	std::string getSequence(unsigned i);
	np_array<MutationTree> getMutationTree() const;
	np_array<IdPhi> getAllPhi() const;
	np_array<double> getPhiOf(const np_array<unsigned> & ids);
	np_array<IdSequence> read(const np_array<unsigned> & ids);
	np_array<IdSequence> readAll();
	unsigned newMutation(Time t, PatchID loc, unsigned i);
	Time nextMutation();
	Vec<unsigned> getParents() const {return parents_;};
private:
	void computeIthSequence(unsigned i);
};

Haplotypes::Haplotypes(RNGcore * rng, std::map<std::string,double> properties) :
			rng_(rng), mr_(properties["mutation_rate"]), seqs_(1), parents_(1,-1), birth_ts_(1,0), birth_locs_(1,-1), phi_h_(1,1) {
	// gamma distribution arbitrarily chosen
	// parametrized with: mean = k * theta, and: variance = k * theta^2
	double k = properties["mutation_k"];
	double theta = 1 / mr_ / k;
	mut_period_ = std::gamma_distribution<>(k, theta);
	// two-point distribution arbitrarily chosen
	double p = properties["fitness_p"];
	double q = 0.5 * (1 - p);
	double deltaM = properties["fitness_delta-"];
	double deltaP = properties["fitness_delta+"];
	phi_shifter_ = DiscreteDistribution<double>({q,q,p}, {deltaM,0,deltaP});
}

Haplotypes::Haplotypes(const np_array<MutationTree> & muttree) :
			seqs_(muttree.shape(0)+1), parents_(muttree.shape(0)+1), birth_ts_(muttree.shape(0)+1), birth_locs_(muttree.shape(0)+1) {
	auto view = muttree.unchecked<1>();
	parents_[0] = -1;
	birth_ts_[0] = -1;
	birth_locs_[0] = -1;
	for (unsigned i = 0; i < muttree.shape(0); ++i){
		parents_[i+1] = view[i].parent;
		birth_ts_[i+1] = view[i].t;
		birth_locs_[i+1] = view[i].loc;
	}
}

double Haplotypes::getMutationRate() const{
	return mr_;
}

double Haplotypes::getPhiH(unsigned i) const{
	return phi_h_[i];
}

unsigned Haplotypes::getTotal() const{
	return seqs_.size();
}

std::string Haplotypes::getSequence(unsigned i){
	computeIthSequence(i);
	return static_cast<std::string>(seqs_[i]);
}

np_array<MutationTree> Haplotypes::getMutationTree() const{
	np_array<MutationTree> tree(parents_.size()-1);
	auto view = tree.mutable_unchecked<1>();
	for (unsigned i = 1; i < parents_.size(); ++i){
		view[i-1].t = birth_ts_[i];
		view[i-1].loc = birth_locs_[i];
		view[i-1].parent = parents_[i];
		view[i-1].child = i;
	}
	return tree;
}

np_array<IdPhi> Haplotypes::getAllPhi() const{
	np_array<IdPhi> allphi(phi_h_.size());
	auto view = allphi.mutable_unchecked<1>();
	for (unsigned i = 0; i < phi_h_.size(); ++i){
		view[i].id = i;
		view[i].phi = phi_h_[i];
	}
	return allphi;
}

np_array<double> Haplotypes::getPhiOf(const np_array<unsigned> & ids){
	np_array<double> phis(ids.shape(0));
	auto view_p = phis.mutable_unchecked<1>();
	auto view_i = ids.unchecked<1>();
	for (unsigned i = 0; i < ids.shape(0); ++i){
		view_p[i] = phi_h_[view_i[i]];
	}
	return phis;
}

np_array<IdSequence> Haplotypes::read(const np_array<unsigned> & ids){
	np_array<IdSequence> sequences(ids.shape(0));
	auto view_seqs = sequences.mutable_unchecked<1>();
	auto view_ids = ids.unchecked<1>();
	computeIthSequence(0);
	for (unsigned i = 0; i < ids.shape(0); ++i){
		unsigned current_id = view_ids[i];
		if (current_id == 0)	{	continue;	}
		if (i > 0){
			if (current_id <= view_ids[i-1]){
				throw std::runtime_error("Array of IDs to read must be sorted and without duplicates");
			}
		}
		unsigned jumps = 0;
		unsigned parent_id = parents_[current_id];
		while (!seqs_[parent_id].isValid()){
			++jumps;
			parent_id = parents_[parent_id];
		}
		Sequence<seq_len> newsequence = seqs_[parent_id].generateMutation(rng_);
		for (unsigned j = 0; j < jumps; ++j){
			newsequence = newsequence.generateMutation(rng_);
		}
		seqs_[current_id] = newsequence;
		view_seqs[i].id = current_id;
		newsequence.writeSequenceInto(view_seqs[i].sequence);
	}
	return sequences;
}

np_array<IdSequence> Haplotypes::readAll(){
	np_array<IdSequence> sequences(seqs_.size());
	auto view_seqs = sequences.mutable_unchecked<1>();
	for (unsigned i = 0; i < seqs_.size(); ++i){
		computeIthSequence(i);
		view_seqs[i].id = i;
		seqs_[i].writeSequenceInto(view_seqs[i].sequence);
	}
	return sequences;
}

unsigned Haplotypes::newMutation(Time t, PatchID loc, unsigned i){
	seqs_.emplace_back();
	birth_ts_.emplace_back(t);
	birth_locs_.emplace_back(loc);
	parents_.emplace_back(i);
	double newphi = phi_h_.at(i);
	if (i)	{	newphi += phi_shifter_(*rng_);	}
	phi_h_.emplace_back(newphi);
	return seqs_.size() - 1;
}

Time Haplotypes::nextMutation(){
	return mut_period_(*rng_);
}

void Haplotypes::computeIthSequence(unsigned i){
	if (!seqs_.at(i).isValid()){
		if (i == 0){
			seqs_[i] = Sequence<seq_len>(rng_);
		}
		else{
			unsigned parent = parents_[i];
			computeIthSequence(parent);
			seqs_[i] = seqs_[parent].generateMutation(rng_);
		}
	}
}

#endif