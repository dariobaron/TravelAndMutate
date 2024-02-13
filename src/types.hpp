#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

template<typename T>
using np_array = py::array_t<T,py::array::c_style>;

using Time = unsigned;

template<typename T>
using Vec = std::vector<T>;

struct FullTraj{
	Time t;
	unsigned S, E, I, R, Enew, Inew;
	FullTraj(Time t, unsigned S, unsigned E, unsigned I, unsigned R, unsigned Enew, unsigned Inew) :
			t(t), S(S), E(E), I(I), R(R), Enew(Enew), Inew(Inew) {}
};

#endif