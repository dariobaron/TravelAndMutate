#include "trees.hpp"

PYBIND11_MODULE(trees, m) {

	py::module types = py::module::import("TravelAndMutate.types");

	py::class_<TreeBalanceProxy>(m, "TreeBalanceProxy")
		.def(py::init<Vec<unsigned>>())
		.def("tree", &TreeBalanceProxy::getTree)
		.def("internals", &TreeBalanceProxy::getInternals)
		.def("tips", &TreeBalanceProxy::getTips);

}
