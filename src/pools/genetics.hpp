#ifndef HAPLOTYPE_HPP
#define HAPLOTYPE_HPP

class Haplotype{
public:
	Haplotype(unsigned n);
	~Haplotype(){}
	double getPhi() const;
	Haplotype& operator=(unsigned n);
	Haplotype& operator+=(unsigned n);
	Haplotype& operator+=(const Haplotype & other);
	Haplotype& operator-=(unsigned n);
	Haplotype& operator-=(const Haplotype & other);
	Haplotype operator-(const Haplotype & other);
};


#endif