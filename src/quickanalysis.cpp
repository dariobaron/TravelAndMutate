#include "quickanalysis.hpp"

PYBIND11_MODULE(quickanalysis, m){

    py::module types = py::module::import("TravelAndMutate.types");

	m.def("computeDepths", &computeDepths);

}