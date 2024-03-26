#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "types.hpp"

struct Individual{
	unsigned id_, infector_id_;
	PatchID patch_, infector_patch_;
	Individual() : id_(-1), infector_id_(-1), patch_(-1), infector_patch_(-1) {}
	Individual(PatchID patch, unsigned id, PatchID inf_patch=-1, unsigned inf_id=-1) :
		id_(id), infector_id_(inf_id), patch_(patch), infector_patch_(inf_patch) {}
	friend struct IndividualCompare;
	bool operator==(const Individual & other){
		return patch_ == other.patch_ && id_ == other.id_;
	}
	void print() const{
		py::print(patch_, id_, infector_patch_, infector_id_);
	}
};

struct IndividualCompare{
	bool operator()(const Individual & one, const Individual & other) const{
		if (one.patch_ == other.patch_){
			return one.id_ < other.id_;
		}
		return one.patch_ < other.patch_;
	}
};


#endif