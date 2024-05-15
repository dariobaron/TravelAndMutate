#include "lookuptable.hpp"

PYBIND11_MODULE(lookuptable, m){

	py::class_<LookupTable>(m, "LookupTable")
		.def(py::init<const np_array<double>&,const np_array<double>&>())
		.def("__call__", py::vectorize(&LookupTable::evaluate))
	;

}