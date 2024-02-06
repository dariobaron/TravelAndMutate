#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <string>
#include <pybind11/pybind11.h>
#include "types.hpp"

namespace py = pybind11;

template<typename T>
void print(Vec<T> v, std::string prefix = ""){
	py::print(prefix, "(", v[0], py::arg("end")="");
	for (unsigned i = 1; i < v.size(); ++i){
		py::print(",", v[i], py::arg("end")="");
	}
	py::print(")", py::arg("flush")=true);
}

#endif