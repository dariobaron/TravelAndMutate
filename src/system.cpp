#include "system.hpp"
#include <pybind11/stl.h>

PYBIND11_MODULE(system, m) {

    m.doc() = "Module including the Epidemic class";

    py::module types = py::module::import("travelAndMutate.types");

    py::class_<System>(m, "System")
        .def(py::init<RNGcore*,const np_array<double>&,Time>(),
                py::arg("rng_ptr"), py::arg("commuting_matrix"), py::arg("dt"))
        .def("setPatchProperties", &System::setPatchProperties<unsigned,double,double,double>)
		.def("seedEpidemic", &System::seedEpidemic<unsigned>)
		.def("spreadForTime", &System::spreadForTime)
		.def("getFullTrajectory", &System::getFullTrajectory);
}