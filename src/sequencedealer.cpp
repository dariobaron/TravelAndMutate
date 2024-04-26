#include "sequencedealer.hpp"

PYBIND11_MODULE(SequenceDealer, m) {

    py::class_<SequenceDealer>(m, "SequenceDealer")
		.def(py::init<>())
		.def("read", &SequenceDealer::read);
}