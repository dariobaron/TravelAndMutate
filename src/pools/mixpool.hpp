#ifndef MIXPOOL_HPP
#define MIXPOOL_HPP

#include "../randomcore.hpp"


class MixPool{
protected:
	PatchID patch_id_;
	double rate_;
	Vec<unsigned> sizes_;
public:
	MixPool(PatchID patch_id, unsigned n=0);
	MixPool(PatchID patch_id, unsigned gamma_trick, double rate);
	unsigned size() const;
	double getPhi() const;
	MixPool& operator+=(const MixPool & other);
	MixPool generate(Time, unsigned n);
	MixPool generate(Time, const MixPool & other);
	void shift(RNGcore * rng);
	MixPool getNewErased(RNGcore *) const;
	MixPool sampleInfectors(RNGcore *, unsigned n) const;
	void moveFromTo(Time, MixPool & source, MixPool & target) const;
	void clear();
};

MixPool::MixPool(PatchID patch_id, unsigned n) : patch_id_(patch_id), sizes_(1,n) {}

MixPool::MixPool(PatchID patch_id, unsigned gamma_trick, double rate) : patch_id_(patch_id), rate_(rate), sizes_(gamma_trick) {}

unsigned MixPool::size() const{
	unsigned size = 0;
	for (auto s : sizes_){
		size += s;
	}
	return size;
}

double MixPool::getPhi() const{
	double phi = 0;
	for (auto s : sizes_){
		phi += s;
	}
	return phi;
}

MixPool& MixPool::operator+=(const MixPool & other){
	sizes_[0] += other.size();
	return *this;
}

MixPool MixPool::generate(Time, unsigned n){
	return MixPool(patch_id_, n);
}

MixPool MixPool::generate(Time, const MixPool & other){
	return other;
}

void MixPool::shift(RNGcore * rng){
	if (sizes_.size() > 1){
		for (int i = sizes_.size()-2; i >= 0; --i){
			std::binomial_distribution Distr(sizes_[i], rate_);
			unsigned n = Distr(*rng);
			sizes_[i+1] += n;
			sizes_[i] -= n;
		}
	}
}

MixPool MixPool::getNewErased(RNGcore * rng) const{
	std::binomial_distribution Distr(sizes_.back(), rate_);
	unsigned n = Distr(*rng);
	return MixPool(patch_id_, n);
}

MixPool MixPool::sampleInfectors(RNGcore *, unsigned n) const{
	return MixPool(patch_id_, n);
}

void MixPool::moveFromTo(Time, MixPool & source, MixPool & target) const{
	target.sizes_[0] += sizes_[0];
	source.sizes_.back() -= sizes_[0];
}

void MixPool::clear(){
	for (auto & s : sizes_){
		s = 0;
	}
}


#endif