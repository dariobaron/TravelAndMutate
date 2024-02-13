#include "system.hpp"
#include <pybind11/stl.h>

PYBIND11_MODULE(system, m) {

    m.doc() = "Module including the Epidemic class";

    py::module types = py::module::import("TravelAndMutate.types");

    py::class_<System>(m, "System")
        .def(py::init<RNGcore*,Time,const np_array<double>&,const np_array<PatchProperties>&>(),
                py::arg("rng_ptr"), py::arg("dt"), py::arg("commuting_matrix"), py::arg("patch_properties"))
		.def("spreadForTime", &System::spreadForTime)
		.def("getFullTrajectory", &System::getFullTrajectory);
}