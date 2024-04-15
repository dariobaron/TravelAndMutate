#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

template<typename T>
using np_array = py::array_t<T,py::array::c_style>;

using Time = unsigned;

using PatchID = unsigned;

template<typename T>
using Vec = std::vector<T>;

struct PatchProperties{
	unsigned N;
	double beta, epsilon, mu;
	unsigned I0;
};

struct FullTraj{
	Time t;
	unsigned S, E, I, R, Enew, Inew;
	FullTraj(Time t, unsigned S, unsigned E, unsigned I, unsigned R, unsigned Enew, unsigned Inew) :
			t(t), S(S), E(E), I(I), R(R), Enew(Enew), Inew(Inew) {}
};

struct InfecTree{
	Time t;
	PatchID loc;
	unsigned ID;
	PatchID inf_loc;
	unsigned inf_ID;
	InfecTree(Time t, PatchID loc, unsigned ID, PatchID inf_loc, unsigned inf_ID) :
			t(t), loc(loc), ID(ID), inf_loc(inf_loc), inf_ID(inf_ID) {}
};

struct MutTree{
	Time t;
	PatchID loc;
	unsigned ID;
	unsigned mut;
	PatchID inf_loc;
	unsigned inf_ID;
	unsigned inf_mut;
	MutTree(Time t, PatchID loc, unsigned ID, unsigned mut, PatchID inf_loc, unsigned inf_ID, unsigned inf_mut) : 
			t(t), loc(loc), ID(ID), mut(mut), inf_loc(inf_loc), inf_ID(inf_ID), inf_mut(inf_mut) {}
};

#endif