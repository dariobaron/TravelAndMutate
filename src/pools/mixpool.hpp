#ifndef MIXPOOL_HPP
#define MIXPOOL_HPP

#include "../randomcore.hpp"


class MixPool{
protected:
	PatchID patch_id_;
	unsigned size_;
public:
	MixPool(PatchID patch_id, unsigned n=0);
	unsigned size() const;
	double getPhi() const;
	MixPool& operator+=(const MixPool & other);
	MixPool generate(unsigned n);
	MixPool generate(const MixPool & other);
	MixPool sample(RNGcore *, unsigned n) const;
	MixPool sampleWithReplacement(RNGcore *, unsigned n) const;
	void moveFromTo(MixPool & source, MixPool & target) const;
	void clear();
};

MixPool::MixPool(PatchID patch_id, unsigned n) : patch_id_(patch_id), size_(n){}

unsigned MixPool::size() const{
	return size_;
}

double MixPool::getPhi() const{
	return size_;
}

MixPool& MixPool::operator+=(const MixPool & other){
	size_ += other.size();
	return *this;
}

MixPool MixPool::generate(unsigned n){
	return MixPool(patch_id_,n);
}

MixPool MixPool::generate(const MixPool & other){
	return other;
}

MixPool MixPool::sample(RNGcore *, unsigned n) const{
	n = std::min(size_, n);
	return MixPool(patch_id_,n);
}

MixPool MixPool::sampleWithReplacement(RNGcore *, unsigned n) const{
	return MixPool(patch_id_,n);
}

void MixPool::moveFromTo(MixPool & source, MixPool & target) const{
	target.size_ += size_;
	source.size_ -= size_;
}

void MixPool::clear(){
	size_ = 0;
}


#endif