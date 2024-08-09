#ifndef MUTPASSIVE_HPP
#define MUTPASSIVE_HPP

#include "../../types.hpp"
#include "../../host.hpp"
#include "../../haplotypes.hpp"
#include "mutdiff.hpp"

class MutPassive{
public:
	friend class MutDiff;
private:
	Haplotypes * haplos_;
	PatchID patch_id_;
	unsigned size_;
	unsigned count_;
public:
	MutPassive(PatchID patch_id, unsigned n=0);
	void setHaplotypes(Haplotypes * seqdealer);
	unsigned size() const;
	MutDiff generate(Time t, unsigned n);
	MutDiff generate(Time t, const MutDiff & other);
};

MutPassive::MutPassive(PatchID patch_id, unsigned n) : patch_id_(patch_id), size_(n), count_(0) {}

void MutPassive::setHaplotypes(Haplotypes * seqdealer){
	haplos_ = seqdealer;
}

unsigned MutPassive::size() const{
	return size_;
}

MutDiff MutPassive::generate(Time t, unsigned n){
	Vec<Host> v_mut(n);
	if (n){
		unsigned newmutation = haplos_->newMutation(t, patch_id_, 0);
		for (auto & i : v_mut){
			Time tnext = t + haplos_->nextMutation();
			i = Host(t, tnext, patch_id_, count_, newmutation);
			++count_;
		}
	}
	return MutDiff(patch_id_, v_mut);
}

MutDiff MutPassive::generate(Time t, const MutDiff & other){
	Vec<Host> v_mut(other.size());
	const Vec<Host> & infectors = other.getHosts();
	for (unsigned i = 0; i < v_mut.size(); ++i){
		Time tnext = t + haplos_->nextMutation();
		v_mut[i] = Host(t, tnext, patch_id_, count_, infectors[i].evolved_mut_, infectors[i].patch_, infectors[i].id_, infectors[i].mut_);
		++count_;
	}
	return MutDiff(patch_id_, v_mut);
}

#endif