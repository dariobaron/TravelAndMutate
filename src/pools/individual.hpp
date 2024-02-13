#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "pool.hpp"

class Individual : public Pool{
public:
	Individual(unsigned n);
	~Individual();
	double getPhi() const;
	Individual& operator=(unsigned n);
	Individual& operator+=(unsigned n);
	Individual& operator+=(const Individual & other);
	Individual& operator-=(unsigned n);
	Individual& operator-=(const Individual & other);
	Individual operator-(const Individual & other);
};

#endif