#include <variable.hpp>
#include <blas.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);


template <template <class> class Var, typename DType>
void definePythonVariable(py::module& m)
{
    py::class_<Var<DType>, std::shared_ptr<Var<DType>>>(m, "F32BlasVariable")
        .def(py::init<Shape, int, int>(), py::arg("shape"), py::arg("value"), py::arg("adjoints") = 0)
        .def("__add__", Bare::BLAS::operator+<DType>, py::is_operator())
        .def_buffer([](Var<DType> &v) -> py::buffer_info {
            return py::buffer_info(
                v.values(),                               /* Pointer to buffer */
                sizeof(DType),                          /* Size of one scalar */
                py::format_descriptor<DType>::format(), /* Python struct-style format descriptor */
                2,                                      /* Number of dimensions */
                { (unsigned long)v.shape().m, (unsigned long)v.shape().n },                 /* Buffer dimensions */
                { sizeof(DType) * v.shape().n,             /* Strides (in bytes) for each index */
                  sizeof(DType) }
            );
        });
}

PYBIND11_PLUGIN(autodiff) {
    py::module m("autodiff", "AutoDiff Python bindings");

    py::class_<Shape>(m, "Shape")
        .def(py::init<int, int>());


    py::class_<Tape>(m, "Tape")
        .def_static("use", Tape::use, py::arg("tape") = (Tape*)nullptr)
        .def("clear", &Tape::clear)
        .def("__enter__", [](const Tape&){})
        .def("__exit__", [](Tape& tape, py::none a, py::none b, py::none c) {
            tape.clear();
        })
        .def("__exit__", [](Tape& tape, py::object a, py::object b, py::object c) {
            tape.clear();
        });

#ifdef WITH_BLAS_SUPPORT

    definePythonVariable<Bare::BLAS::Variable, float>(m);

#endif

    return m.ptr();
}
