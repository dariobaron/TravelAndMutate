#include "trees.hpp"

PYBIND11_MODULE(trees, m) {

	py::module types = py::module::import("TravelAndMutate.types");

	py::class_<TreeBalanceProxy>(m, "TreeBalanceProxy")
		.def(py::init<Vec<unsigned>>())
		.def("tree", &TreeBalanceProxy::getTree)
		.def("internals", &TreeBalanceProxy::getInternals)
		.def("tips", &TreeBalanceProxy::getTips);
	
	py::class_<PyTree>(m, "Tree")
		.def(py::init<const np_array<ParentChild>&>())
		.def("computeDepths", &PyTree::getDepths)
		.def("computeProbabilities", &PyTree::getProbabilities)
		.def("computeB2", &PyTree::getB2)
		.def("computeB2Norm", &PyTree::getB2Norm)
		.def("generateYuleEdges", &PyTree::getYuleEdges)
	;

}
