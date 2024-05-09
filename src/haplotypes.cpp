#include "haplotypes.hpp"
#include <pybind11/stl.h>

PYBIND11_MODULE(haplotypes, m) {
	
    py::module types = py::module::import("TravelAndMutate.types");

    py::class_<Haplotypes>(m, "Haplotypes")
		.def(py::init<RNGcore*,double>())
		.def("getTotal", &Haplotypes::getTotal)
		.def("getSequence", &Haplotypes::getSequence)
		.def("getMutationTree", &Haplotypes::getMutationTree)
		.def("read", &Haplotypes::read)
		.def("readAll", &Haplotypes::readAll)
		;
}