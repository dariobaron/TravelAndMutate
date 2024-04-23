#include "eventrecorder.hpp"

PYBIND11_MODULE(eventrecorder, m){

	PYBIND11_NUMPY_DTYPE(StateTransition, t, loc, ID, newstate);

	py::class_<EventRecorder>(m, "EventRecorder")
		.def(py::init<>())
		.def("read", &EventRecorder::read);
}
