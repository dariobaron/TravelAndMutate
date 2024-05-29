#ifndef randomcore_h
#define randomcore_h

#include <random>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <numpy/random/bitgen.h>

namespace py = pybind11;

uint32_t next_uint32(void * st);
uint64_t next_uint64(void * st);
double next_double(void * st);

class RNGcore{
public:
	using result_type = std::mt19937_64::result_type;

protected:
	std::mt19937_64 rng_;

public:
	RNGcore() : rng_() {};
	RNGcore(unsigned long seed) : rng_(seed) {};

	void setup_numpy_capsule(py::capsule capsule){
		bitgen_t * np_bg(capsule);
		np_bg->state = &rng_;
		np_bg->next_uint64 = next_uint64;
		np_bg->next_uint32 = next_uint32;
		np_bg->next_double = next_double;
		np_bg->next_raw = next_uint64;
	};


	auto operator()(){
		return rng_();
	};


	auto max() const{
		return rng_.max();
	}
	auto min() const{
		return rng_.min();
	}

	auto & get(){
		return rng_;
	}

};


uint32_t next_uint32(void * st){
	RNGcore * rng = static_cast<RNGcore*>(st);
	return (uint32_t)(rng->operator()());
}

uint64_t next_uint64(void * st){
	RNGcore * rng = static_cast<RNGcore*>(st);
	return (uint64_t)(rng->operator()());
}

double next_double(void * st){
	return next_uint64(st) / ((double) std::numeric_limits<uint64_t>::max()+1.);
}


#endif