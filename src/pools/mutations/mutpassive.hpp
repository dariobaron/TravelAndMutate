#ifndef MUTPASSIVE_HPP
#define MUTPASSIVE_HPP

#include "../../types.hpp"
#include "../../host.hpp"
#include "../../allmutations.hpp"
#include "mutdiff.hpp"

class MutPassive{
public:
	static AllMutations allmutations;
	friend class MutDiff;
private:
	PatchID patch_id_;
	unsigned size_;
	unsigned count_;
public:
	MutPassive(PatchID patch_id, unsigned n=0);
	unsigned size() const;
	MutDiff generate(Time t, unsigned n);
	MutDiff generate(Time t, const MutDiff & other);
};

AllMutations MutPassive::allmutations;

MutPassive::MutPassive(PatchID patch_id, unsigned n) : patch_id_(patch_id), size_(n), count_(0) {}

unsigned MutPassive::size() const{
	return size_;
}

MutDiff MutPassive::generate(Time t, unsigned n){
	Vec<Host> v_mut(n);
	for (auto & i : v_mut){
		Time tnext = t + allmutations.nextMutation();
		i = Host(t, tnext, patch_id_, count_, allmutations.newMutation());
		++count_;
	}
	return MutDiff(patch_id_, v_mut);
}

MutDiff MutPassive::generate(Time t, const MutDiff & other){
	Vec<Host> v_mut(other.size());
	const Vec<Host> & infectors = other.getHosts();
	for (unsigned i = 0; i < v_mut.size(); ++i){
		Time tnext = t + allmutations.nextMutation();
		v_mut[i] = Host(t, tnext, patch_id_, count_, infectors[i].evolved_mut_, infectors[i].patch_, infectors[i].id_, infectors[i].mut_);
		++count_;
	}
	return MutDiff(patch_id_, v_mut);
}

#endif