#include "recorder.hpp"

PYBIND11_MODULE(recorder, m){

	py::module types = py::module::import("TravelAndMutate.types");
	py::module trees = py::module::import("TravelAndMutate.trees");

	py::class_<Recorder<Mix>>(m, "RecorderMix")
		.def(py::init<unsigned>())
		.def("getFullTrajectory", &Recorder<Mix>::getFullTrajectory)
		.def("getInfectionTree", &Recorder<Mix>::getInfectionTree)
	;

	py::class_<Recorder<Individuals>>(m, "RecorderIndividuals")
		.def(py::init<unsigned>())
		.def("getFullTrajectory", &Recorder<Individuals>::getFullTrajectory)
		.def("getInfectionTree", &Recorder<Individuals>::getInfectionTree)
	;

	py::class_<Recorder<Mutations>>(m, "RecorderMutations")
		.def(py::init<unsigned>())
		.def("getFullTrajectory", &Recorder<Mutations>::getFullTrajectory)
		.def("getInfectionTree", &Recorder<Mutations>::getInfectionTree)
	;

}