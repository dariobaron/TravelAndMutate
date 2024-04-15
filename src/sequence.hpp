#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <bitset>
#include <string>
#include <exception>
#include <random>
#include "randomcore.hpp"

template<unsigned LEN>
class Sequence{
	std::bitset<2*LEN> seq_;
public:
	Sequence() {};
	Sequence(std::string str);
	operator std::string() const;
	void randomize(RNGcore * rng);
};


template<unsigned LEN>
Sequence<LEN>::Sequence(std::string str) : seq_(LEN) {
	if (str.size() != LEN){
		throw std::runtime_error("Sequence string of wrong length");
	}
	for (unsigned i = 0; i < LEN; ++i){
		switch (str[i]){
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
}

template<unsigned LEN>
Sequence<LEN>::operator std::string() const{
	std::string decoded(LEN, '0');
	for (unsigned i = 0; i < LEN; ++i){
		switch (seq_[2*i] + 2*seq_[2*i+1]){
		case 0:
			decoded[i] = 'A';
			break;
		case 1:
			decoded[i] = 'C';
			break;
		case 2:
			decoded[i] = 'G';
			break;
		case 3:
			decoded[i] = 'T';
			break;
		}
	}
	return decoded;
}

template<unsigned LEN>
void Sequence<LEN>::randomize(RNGcore * rng){
	std::bernoulli_distribution Distr(0.5);
	for (auto & b : seq_){
		b = Distr(*rng);
	}
}

#endif