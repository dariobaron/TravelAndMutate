#include "system.hpp"
#include <pybind11/stl.h>

PYBIND11_MODULE(system, m) {

    m.doc() = "Module including the Epidemic class";

    py::module types = py::module::import("TravelAndMutate.types");
	py::module trees = py::module::import("TravelAndMutate.trees");
	py::module haplotypes = py::module::import("TravelAndMutate.haplotypes");

    py::class_<System<Mix>>(m, "SystemMix")
		.def(py::init<RNGcore*,const np_array<double>&,const np_array<PatchProperties>&,unsigned>(),
				py::arg("rng_ptr"), py::arg("commuting_matrix"), py::arg("patch_properties"), py::arg("gamma_trick"))
		.def("setVerbosity", &System<Mix>::setVerbosity)
		.def("seedEpidemic", &System<Mix>::seedEpidemic)
		.def("spreadForTime", &System<Mix>::spreadForTime)
		.def("getFullTrajectory", &System<Mix>::getFullTrajectory);

    py::class_<System<Individuals>>(m, "SystemIndividuals")
		.def(py::init<RNGcore*,const np_array<double>&,const np_array<PatchProperties>&,unsigned>(),
				py::arg("rng_ptr"), py::arg("commuting_matrix"), py::arg("patch_properties"), py::arg("gamma_trick"))
		.def("setVerbosity", &System<Individuals>::setVerbosity)
		.def("seedEpidemic", &System<Individuals>::seedEpidemic)
		.def("spreadForTime", &System<Individuals>::spreadForTime)
		.def("getFullTrajectory", &System<Individuals>::getFullTrajectory)
		.def("getInfectionTree", &System<Individuals>::getInfectionTree)
		.def("getTreeBalance", &System<Individuals>::getTreeBalance);

    py::class_<System<Mutations>>(m, "SystemMutations")
		.def(py::init<RNGcore*,const np_array<double>&,const np_array<PatchProperties>&,unsigned>(),
				py::arg("rng_ptr"), py::arg("commuting_matrix"), py::arg("patch_properties"), py::arg("gamma_trick"))
		.def("setVerbosity", &System<Mutations>::setVerbosity)
		.def("seedEpidemic", &System<Mutations>::seedEpidemic)
		.def("spreadForTime", &System<Mutations>::spreadForTime)
		.def("getFullTrajectory", &System<Mutations>::getFullTrajectory)
		.def("getMutationTree", &System<Mutations>::getMutationTree)
		.def("getMutPerHost", &System<Mutations>::getMutPerHost)
		.def("setHaplotypes", &System<Mutations>::setHaplotypes)
		;
}