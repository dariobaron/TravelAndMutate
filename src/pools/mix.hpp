#ifndef POOL_HPP
#define POOL_HPP

class Mix{
protected:
	unsigned size_;
public:
	Mix();
	Mix(unsigned n);
	virtual ~Mix();
	unsigned size() const;
	virtual double getPhi() const;
	virtual Mix& operator=(unsigned n);
	virtual Mix& operator+=(unsigned n);
	virtual Mix& operator+=(const Mix & other);
	virtual Mix& operator-=(unsigned n);
	virtual Mix& operator-=(const Mix & other);
	virtual Mix sample(unsigned Enew) const;
	virtual void clear();
};

Mix::Mix() : size_(0){}

Mix::Mix(unsigned n) : size_(n){}

Mix::~Mix(){}

unsigned Mix::size() const{
	return size_;
}

double Mix::getPhi() const{
	return size_;
}

Mix& Mix::operator=(unsigned n){
	size_ = n;
	return *this;
}

Mix& Mix::operator+=(unsigned n){
	size_ += n;
	return *this;
}

Mix& Mix::operator+=(const Mix & other){
	size_ += other.size_;
	return *this;
}

Mix& Mix::operator-=(unsigned n){
	size_ -= n;
	return *this;
}

Mix& Mix::operator-=(const Mix & other){
	size_ -= other.size_;
	return *this;
}

Mix Mix::sample(unsigned Enew) const{
	return Mix(Enew);
}

void Mix::clear(){
	size_ = 0;
}


#endif