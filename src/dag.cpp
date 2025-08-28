#include "dag.hpp"


PYBIND11_MODULE(dag, m) {

	py::module types = py::module::import("TravelAndMutate.types");

	m.def("computeSubnetSizes", &computeSubnetSizes, "Compute the sizes of all subnets in a DAG represented by an edgelist", py::arg("edgelist"), py::arg("roots"));

}