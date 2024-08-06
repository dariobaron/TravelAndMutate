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
		.def("computeWidths", &PyTree::getWidths)
		.def("computeProbabilities", &PyTree::getProbabilities)
		.def("computeNLeavesSubtree", &PyTree::getNLeavesSubtree)
		.def("computeB2", &PyTree::getB2)
		.def("computeB2Norm", &PyTree::getB2Norm)
		.def("computeCophenetic", &PyTree::getCophenetic)
		.def("computeCopheneticNorm", &PyTree::getCopheneticNorm)
		.def("computeNChildrenPerNode", &PyTree::getNChildren)
		.def("subset", &PyTree::subset)
		.def("generateYuleEdges", &PyTree::getYuleEdges)
		.def("getRandomizedEdges", &PyTree::getRandomizedEdges)
	;

}
