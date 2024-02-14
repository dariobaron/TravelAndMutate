#ifndef POOL_HPP
#define POOL_HPP

class Pool{
protected:
	unsigned size_;
public:
	Pool(unsigned n);
	virtual ~Pool();
	unsigned size() const;
	virtual double getPhi() const;
	virtual Pool& operator=(unsigned n);
	virtual Pool& operator+=(unsigned n);
	virtual Pool& operator+=(const Pool & other);
	virtual Pool& operator-=(unsigned n);
	virtual Pool& operator-=(const Pool & other);
	virtual Pool sample(unsigned Enew) const;
	virtual void clear();
};

Pool::Pool(unsigned n) : size_(n){}

Pool::~Pool(){}

unsigned Pool::size() const{
	return size_;
}

double Pool::getPhi() const{
	return size_;
}

Pool& Pool::operator=(unsigned n){
	size_ = n;
	return *this;
}

Pool& Pool::operator+=(unsigned n){
	size_ += n;
	return *this;
}

Pool& Pool::operator+=(const Pool & other){
	size_ += other.size_;
	return *this;
}

Pool& Pool::operator-=(unsigned n){
	size_ -= n;
	return *this;
}

Pool& Pool::operator-=(const Pool & other){
	size_ -= other.size_;
	return *this;
}

Pool Pool::sample(unsigned Enew) const{
	return Pool(Enew);
}

void Pool::clear(){
	size_ = 0;
}


#endif