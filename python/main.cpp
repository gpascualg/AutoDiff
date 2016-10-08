#include <pybind11/pybind11.h>
#include <variable.hpp>
#include <blas.hpp>

namespace py = pybind11;

PYBIND11_PLUGIN(autodiff) {
    py::module m("autodiff", "AutoDiff Python bindings");

#ifdef WITH_BLAS_SUPPORT
    using F32BlasVariable = Bare::BLAS::Variable<float>;

    py::class_<F32BlasVariable>(m, "F32BlasVariable")
        .def(py::init<int, int>());
#endif

    return m.ptr();
}
