#include "haplotypes.hpp"
#include <pybind11/stl.h>

PYBIND11_MODULE(haplotypes, m) {
	
    py::module types = py::module::import("TravelAndMutate.types");

    py::class_<Haplotypes>(m, "Haplotypes")
		.def(py::init<RNGcore*,std::map<std::string,double>>())
		.def(py::init<const np_array<MutationTree>&>())
		.def(py::init<RNGcore*,const np_array<MutationTree>&>())
		.def("getTotal", &Haplotypes::getTotal)
		.def("getSequence", &Haplotypes::getSequence)
		.def("getMutationTree", &Haplotypes::getMutationTree)
		.def("getAllPhi", &Haplotypes::getAllPhi)
		.def("getPhiOf", &Haplotypes::getPhiOf)
		.def("read", &Haplotypes::read)
		.def("readAll", &Haplotypes::readAll)
		;
}