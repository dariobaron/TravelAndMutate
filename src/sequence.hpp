#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <vector>
#include <string>
#include <exception>
#include <random>
#include "randomcore.hpp"

template<unsigned LEN>
class Sequence{
	Vec<bool> seq_;
	bool valid_;
public:
	Sequence();
	Sequence(RNGcore * rng);
	Sequence(std::string str);
	operator std::string() const;
	bool operator<(const Sequence & other) const;
	void writeSequenceInto(char * ptr) const;
	Sequence generateMutation(RNGcore * rng) const;
	bool isValid() const;
private:
	char getCharFromBits(unsigned i) const;
	void setBits(unsigned i, unsigned val);
	void setBits(unsigned i, char val);
};


template<unsigned LEN>
Sequence<LEN>::Sequence() : valid_(false) {}

template<unsigned LEN>
Sequence<LEN>::Sequence(RNGcore * rng) : seq_(2*LEN), valid_(true) {
	std::bernoulli_distribution Distr(0.5);
	for (unsigned i = 0; i < seq_.size(); ++i){
		seq_[i] = Distr(*rng);
	}
}

template<unsigned LEN>
Sequence<LEN>::Sequence(std::string str) : seq_(2*LEN), valid_(true) {
	if (str.size() != LEN){
		throw std::runtime_error("Sequence string of wrong length");
	}
	for (unsigned i = 0; i < LEN; ++i){
		setBits(i, str[i]);
	}
}

template<unsigned LEN>
Sequence<LEN>::operator std::string() const{
	if (seq_.size() == 0){
		return "";
	}
	std::string decoded(LEN, '\0');
	for (unsigned i = 0; i < LEN; ++i){
		decoded[i] = getCharFromBits(i);
	}
	return decoded;
}

template<unsigned LEN>
bool Sequence<LEN>::operator<(const Sequence & other) const{
	if ((!valid_) || (!other.valid_)){
		return false;
	}
	for (unsigned i = 0; i < LEN; ++i){
		char base1 = getCharFromBits(i);
		char base2 = other.getCharFromBits(i);
		if (base1 != base2){
			return base1 < base2;
		}
	}
	return false;
}

template<unsigned LEN>
void Sequence<LEN>::writeSequenceInto(char * ptr) const{
	for (unsigned i = 0; i < LEN; ++i){
		ptr[i] = getCharFromBits(i);
	}
}

template<unsigned LEN>
Sequence<LEN> Sequence<LEN>::generateMutation(RNGcore * rng) const{
	Sequence<LEN> child(*this);
	std::uniform_int_distribution<> Distr1(0,LEN-1);
	unsigned to_flip = Distr1(rng->get());
	unsigned value = child.seq_[2*to_flip] + 2*child.seq_[2*to_flip+1];
	std::uniform_int_distribution<> Distr2(1,3);
	value += Distr2(rng->get());
	value %= 4;
	child.setBits(to_flip, value);
	return child;
}

template<unsigned LEN>
bool Sequence<LEN>::isValid() const{
	return valid_;
}

template<unsigned LEN>
char Sequence<LEN>::getCharFromBits(unsigned i) const{
	char chr;
	switch (seq_[2*i] + 2*seq_[2*i+1]){
	case 0:
		chr = 'A';
		break;
	case 1:
		chr = 'C';
		break;
	case 2:
		chr = 'G';
		break;
	case 3:
		chr = 'T';
		break;
	}
	return chr;
}

template<unsigned LEN>
void Sequence<LEN>::setBits(unsigned i, unsigned val){
	switch (val){
	case 0:
		seq_[2*i] = 0; seq_[2*i+1] = 0;
		break;
	case 1:
		seq_[2*i] = 1; seq_[2*i+1] = 0;
		break;
	case 2:
		seq_[2*i] = 0; seq_[2*i+1] = 1;
		break;
	case 3:
		seq_[2*i] = 1; seq_[2*i+1] = 1;
		break;
	default:
		throw std::runtime_error("Sequence bits cannot be set with integers >= 4");
	}
}

template<unsigned LEN>
void Sequence<LEN>::setBits(unsigned i, char val){
	switch (val){
	case 'A':
		seq_[2*i] = 0; seq_[2*i+1] = 0;
		break;
	case 'C':
		seq_[2*i] = 1; seq_[2*i+1] = 0;
		break;
	case 'G':
		seq_[2*i] = 0; seq_[2*i+1] = 1;
		break;
	case 'T':
		seq_[2*i] = 1; seq_[2*i+1] = 1;
		break;
	default:
		throw std::runtime_error("Sequence has a wrong basis (not one of A-C-G-T)");
	}
}

#endif