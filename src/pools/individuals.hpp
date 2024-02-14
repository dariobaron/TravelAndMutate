#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "../types.hpp"

class Reservoir{
public:
	Reservoir(unsigned n);
};

class Individuals{
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


Individuals::Individuals(unsigned n, unsigned nmax){}


#endif