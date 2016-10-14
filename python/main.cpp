#include <variable.hpp>
#include <blas.hpp>

#include <algorithm>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>


namespace py = pybind11;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);


template <typename DType>
class PyVariable : public Bare::Variable<DType> {
public:
    using Bare::Variable<DType>::Variable;
    void flag() override { PYBIND11_OVERLOAD(void, Bare::Variable<DType>, flag,); }
};

template <template <class> class Var, typename DType>
void definePythonVariable(py::module& m, py::class_<Tape>& tape, const char* name = "Variable")
{
    py::class_<Bare::Constant<DType>, std::shared_ptr<Bare::Constant<DType>>>(m, "Constant")
        .def(py::init<DType>(), py::arg("value"))
        .def_property_readonly("value", [](Bare::Constant<DType>& constant){
            return constant.value();
        });

    py::class_<Bare::Variable<DType>, PyVariable<DType>> baseVariable(m, "Variable");
    baseVariable
        .def("flag", &Bare::Variable<DType>::flag)
        .def_property_readonly("shape", [](Bare::Variable<DType>& var){
            return std::tuple<int, int> { var.shape().m, var.shape().n };
        });

    py::class_<Var<DType>, std::shared_ptr<Var<DType>>>(m, name, baseVariable)
        .def(py::init<Shape, DType, DType>(), py::arg("shape"), py::arg("value"), py::arg("adjoints") = 0)

        // 2 Dims tuple
        .def("__init__", [](Var<DType>& instance, std::tuple<int, int> s, DType v, DType a){
            new (&instance) Var<DType>(Shape { std::get<0>(s), std::get<1>(s) }, v, a);
        }, py::arg("shape"), py::arg("value"), py::arg("adjoints") = 0)

        // ND list
        .def("__init__", [](Var<DType>& instance, std::vector<int> s, DType v, DType a){
            new (&instance) Var<DType>(Shape { s[0], s[1] }, v, a);
        }, py::arg("shape"), py::arg("value"), py::arg("adjoints") = 0)

        .def("__add__", Bare::BLAS::operator+<DType>, py::is_operator())
        .def("__sub__", Bare::BLAS::operator-<DType>, py::is_operator())
        .def("__mul__", Bare::BLAS::operator*<DType>, py::is_operator())
        .def("__pow__", Bare::BLAS::pow<DType, DType>, py::is_operator())
        .def("__div__", (std::shared_ptr<Var<DType>> (*)(std::shared_ptr<Var<DType>> a, std::shared_ptr<Var<DType>> b))&Bare::BLAS::operator/<DType>, py::is_operator())
        .def("__div__", (std::shared_ptr<Var<DType>> (*)(std::shared_ptr<Var<DType>> a, std::shared_ptr<Bare::Constant<DType>> b))&Bare::BLAS::operator/<DType>, py::is_operator())

        .def_property_readonly("T", [](std::shared_ptr<Var<DType>> instance) {
            return Bare::BLAS::transpose(instance);
        })

        .def_buffer([](Var<DType> &v) {
            return py::buffer_info(
                v.values(),
                sizeof(DType),
                py::format_descriptor<DType>::format(),
                2,
                {
                    (unsigned long)v.shape().m,
                    (unsigned long)v.shape().n
                },
                {
                    sizeof(DType) * v.shape().n,
                    sizeof(DType)
                }
            );
        });

    m.def("tranpose", &Bare::BLAS::transpose<DType>, py::arg("variable"));
    m.def("mul", &Bare::BLAS::mul<DType>, py::arg("a"), py::arg("b"));
    m.def("sum", &Bare::BLAS::sum<DType>, py::arg("variable"));
    m.def("sqrt", &Bare::BLAS::sqrt<DType>, py::arg("variable"));
    m.def("power", &Bare::BLAS::pow<DType, DType>, py::arg("variable"), py::arg("power"));
    m.def("Adjoints", [](Bare::Variable<DType>& v){
            return py::array {
                {
                    (unsigned long)v.shape().m,
                    (unsigned long)v.shape().n
                },
                {
                    sizeof(DType) * v.shape().n,
                    sizeof(DType)
                },
                v.adjoints()
            };
        });

    tape.def("execute", [] (Tape& tape, std::vector<std::shared_ptr<Var<DType>>> targets) {
        std::vector<std::shared_ptr<TapeVariable>> other(targets.size());

        for (auto v : targets)
        {
            other.emplace_back(std::dynamic_pointer_cast<TapeVariable>(v));
        }

        tape.execute(other);
    });
}

PYBIND11_PLUGIN(autodiff) {
    py::module m("autodiff", "AutoDiff Python bindings");

    py::class_<Shape>(m, "Shape")
        .def(py::init<int, int>());


    py::class_<Tape> tape(m, "Tape");
    tape
        .def("__init__", [](Tape& tape, Tape* which) {
            Tape::use(which, &tape);
        }, py::arg("tape") = (Tape*)nullptr)
        .def("__enter__", [](const Tape& tape) -> const Tape& {
            return tape;
        })
        .def("__exit__", [](Tape& tape, py::none a, py::none b, py::none c) {
            tape.clear();
        })
        .def("__exit__", [](Tape& tape, py::object a, py::object b, py::object c) {
            tape.clear();
        })

        .def("execute", (void (Tape::*)())&Tape::execute)
        //.def("execute", (void (Tape::*)(std::vector<std::shared_ptr<TapeVariable>>))&Tape::execute)
        .def("clear", &Tape::clear)
        .def("close", &Tape::close);


#ifdef WITH_BLAS_SUPPORT

    py::module f32 = m.def_submodule("f32");
    py::module f64 = m.def_submodule("f64");

    py::module f32Blas = f32.def_submodule("blas");
    py::module f64Blas = f64.def_submodule("blas");

    definePythonVariable<Bare::BLAS::Variable, float>(f32Blas, tape);
    definePythonVariable<Bare::BLAS::Variable, double>(f64Blas, tape);

#endif

    return m.ptr();
}
