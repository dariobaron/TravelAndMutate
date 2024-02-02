#include "randomcore.hpp"



PYBIND11_MODULE(randomcore, m) {
    m.doc() = "Module including the RNGcore class";

    py::class_<RNGcore>(m, "RNGcore")
        .def(py::init<>())
        .def(py::init<unsigned long>())
        .def("setup_numpy_capsule", &RNGcore::setup_numpy_capsule)
        .def("__call__", &RNGcore::operator());

}