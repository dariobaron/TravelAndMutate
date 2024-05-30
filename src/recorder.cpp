#include "recorder.hpp"

PYBIND11_MODULE(recorder, m){

	py::module types = py::module::import("TravelAndMutate.types");
	py::module trees = py::module::import("TravelAndMutate.trees");

	py::class_<Recorder>(m, "Recorder")
		.def(py::init<unsigned>())
		.def("getFullTrajectory", &Recorder::getFullTrajectory)
		.def("getInfectionTree", &Recorder::getInfectionTree)
		.def("getMutationTree", &Recorder::getMutationTree)
	;

}