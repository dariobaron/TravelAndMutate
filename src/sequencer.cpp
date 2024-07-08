#include "sequencer.hpp"

PYBIND11_MODULE(sequencer, m){

	py::class_<Sequencer>(m, "Sequencer")
		.def(py::init<RNGcore*,double,Time>())
		.def("getSampledIDs", &Sequencer::getSampledIDs)
	;


}