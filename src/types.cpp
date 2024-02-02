#include "types.hpp"

PYBIND11_MODULE(types, m) {

    m.doc() = "Module include Numpy dtypes used in the project.";

    PYBIND11_NUMPY_DTYPE(FullTraj, t, S, E, I, R, Enew, Inew);

}