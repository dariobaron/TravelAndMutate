#ifndef HAPLOTYPES_HPP
#define HAPLOTYPES_HPP

#include <vector>
#include <string>
#include <random>
#include <pybind11/stl.h>
#include "types.hpp"
#include "randomcore.hpp"
#include "sequence.hpp"


class Haplotypes{
private:
	RNGcore * rng_;
	double mr_;
	Vec<Sequence<seq_len>> seqs_;
	Vec<unsigned> parents_;
	Vec<double> phi_h_;
	std::gamma_distribution<> mut_period_;
public:
	Haplotypes(RNGcore * rng, double mr);
	double getMutationRate() const;
	double getPhiH(unsigned i) const;
	unsigned getTotal() const;
	std::string getSequence(unsigned i);
	np_array<ParentChild> getMutationTree() const;
	np_array<IdSequence> read(const np_array<unsigned> & ids);
	np_array<IdSequence> readAll();
	unsigned newMutation(unsigned i);
	Time nextMutation();
	Vec<unsigned> getParents() const {return parents_;};
	Time exportMutPeriod() {return std::round(mut_period_(rng_->get()));};
private:
	void computeIthSequence(unsigned i);
	void checkSequence(unsigned i);
};

Haplotypes::Haplotypes(RNGcore * rng, double mr) : rng_(rng), mr_(mr) {
	// gamma distribution arbitrarily chosen
	// parametrized with: mean = k * theta, and: variance = k * theta^2
	double k = 10; // arbitrarily chosen to preserve the shape
	double theta = 1 / mr_ / k;
	mut_period_ = std::gamma_distribution<>(k, theta);
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
	checkSequence(i);
	computeIthSequence(i);
	return static_cast<std::string>(seqs_[i]);
}

np_array<ParentChild> Haplotypes::getMutationTree() const{
	np_array<ParentChild> tree(parents_.size());
	auto view = tree.mutable_unchecked<1>();
	for (unsigned i = 0; i < parents_.size(); ++i){
		view[i].parent = parents_[i];
		view[i].child = i;
	}
	return tree;
}

np_array<IdSequence> Haplotypes::read(const np_array<unsigned> & ids){
	np_array<IdSequence> sequences(ids.shape(0));
	auto view_seqs = sequences.mutable_unchecked<1>();
	auto view_ids = ids.unchecked<1>();
	for (unsigned i = 0; i < ids.shape(0); ++i){
		checkSequence(view_ids[i]);
		computeIthSequence(view_ids[i]);
		view_seqs[i].id = view_ids[i];
		seqs_[view_ids[i]].writeSequenceInto(view_seqs[i].sequence);
	}
	return sequences;
}

np_array<IdSequence> Haplotypes::readAll(){
	np_array<IdSequence> sequences(seqs_.size());
	auto view_seqs = sequences.mutable_unchecked<1>();
	for (unsigned i = 0; i < seqs_.size(); ++i){
		checkSequence(i);
		computeIthSequence(i);
		view_seqs[i].id = i;
		seqs_[i].writeSequenceInto(view_seqs[i].sequence);
	}
	return sequences;
}

unsigned Haplotypes::newMutation(unsigned i){
	seqs_.emplace_back();
	parents_.emplace_back(i);
	phi_h_.emplace_back(1);	// fitness of the haplotype
	return seqs_.size()-1;
}

Time Haplotypes::nextMutation(){
	return std::round(mut_period_(rng_->get()));
}

void Haplotypes::computeIthSequence(unsigned i){
	if (!seqs_[i].isValid()){
		unsigned parent = parents_[i];
		if (parent == -1){
			seqs_[i] = Sequence<seq_len>(rng_);
		}
		else{
			computeIthSequence(parent);
			seqs_[i] = seqs_[parent].generateMutation(rng_);
		}
	}
}


void Haplotypes::checkSequence(unsigned i){
	if (i >= seqs_.size()){
		throw std::out_of_range("There are not enough sequences (which are "+std::to_string(seqs_.size())+")");
	}
}

#endif