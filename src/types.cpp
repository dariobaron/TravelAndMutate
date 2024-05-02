#include "types.hpp"

PYBIND11_MODULE(types, m) {

	m.doc() = "Module include Numpy dtypes used in the project.";

	PYBIND11_NUMPY_DTYPE(PatchProperties, N, beta, epsilon, mu, I0);

	PYBIND11_NUMPY_DTYPE(FullTraj, t, S, E, I, R, Enew, Inew);

	PYBIND11_NUMPY_DTYPE(InfecTree, t, loc, ID, inf_loc, inf_ID);

	PYBIND11_NUMPY_DTYPE(MutTree, t, loc, ID, mut, inf_loc, inf_ID, inf_mut);

	PYBIND11_NUMPY_DTYPE(IdSequence, id, sequence);
}