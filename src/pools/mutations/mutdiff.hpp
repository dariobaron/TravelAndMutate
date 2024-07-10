#ifndef MUTDIFF_HPP
#define MUTDIFF_HPP

#include "../../types.hpp"
#include "../../host.hpp"
#include "../../sequencer.hpp"

class MutPassive;
class MutActive;

class MutDiff{
private:
	Sequencer * sequencer_;
	PatchID patch_id_;
	Vec<Host> hosts_;
	Vec<unsigned> indices_;
public:
	MutDiff(PatchID patch_id);
	MutDiff(PatchID patch_id, Vec<Host> v);
	MutDiff(PatchID patch_id, Vec<unsigned> v);
	void setSequencer(Sequencer * sequencer);
	unsigned size() const;
	const Vec<Host>& getHosts() const;
	MutDiff& operator+=(const MutDiff & other);
	void clear();
	void moveFromTo(MutPassive & source, MutActive & target) const;
	void moveFromTo(MutActive & source, MutActive & target) const;
	void moveFromTo(MutActive & source, MutPassive & target) const;
};

MutDiff::MutDiff(PatchID patch_id) : sequencer_(nullptr), patch_id_(patch_id) {}

MutDiff::MutDiff(PatchID patch_id, Vec<Host> v) : sequencer_(nullptr), patch_id_(patch_id), hosts_(v) {}

MutDiff::MutDiff(PatchID patch_id, Vec<unsigned> v) : sequencer_(nullptr), patch_id_(patch_id), indices_(v) {}

void MutDiff::setSequencer(Sequencer * sequencer){
	sequencer_ = sequencer;
}

unsigned MutDiff::size() const{
	return std::max(hosts_.size(), indices_.size());
}

const Vec<Host>& MutDiff::getHosts() const{
	return hosts_;
}

MutDiff& MutDiff::operator+=(const MutDiff & other){
	hosts_.insert(hosts_.end(), other.hosts_.begin(), other.hosts_.end());
	indices_.insert(indices_.end(), other.indices_.begin(), other.indices_.end());
	return *this;
}

void MutDiff::clear(){
	hosts_.clear();
	indices_.clear();
}



#endif