#include "quickanalysis.hpp"

PYBIND11_MODULE(quickanalysis, m){

	PYBIND11_NUMPY_DTYPE(IdDepth, id, depth);

    py::module types = py::module::import("TravelAndMutate.types");

	m.def("computeDepths", &computeDepths);

}