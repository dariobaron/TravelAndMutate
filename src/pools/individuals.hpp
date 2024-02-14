#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "../types.hpp"
#include "pool.hpp"

class Reservoir : public Pool{
public:
	Reservoir(unsigned n);
};

class Individuals : public Pool{
protected:
	Vec<unsigned> ids_;
public:
	Individuals(unsigned n, unsigned patch_id);
	~Individuals();
	double getPhi() const;
	Individuals& operator=(unsigned n);
	Individuals& operator+=(unsigned n);
	Individuals& operator+=(const Individuals & other);
	Individuals& operator-=(unsigned n);
	Individuals& operator-=(const Individuals & other);
	Individuals operator-(const Individuals & other);
};


Individuals::Individuals(unsigned n, unsigned nmax) : Pool(n){}


#endif