#include "variable/variable.hpp"
#include "tape/tape.hpp"


template <typename T>
Variable<T>::Variable(T value) : 
    _value{ value }
{}

template <typename T>
SharedVariable<T> operator+(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    auto result = make_variable<T>(var1->raw() + var2->raw());
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        Tape<T>::adjoint(var1)->_value += Tape<T>::adjoint(result)->_value;
        Tape<T>::adjoint(var2)->_value += Tape<T>::adjoint(result)->_value;
    });

    return result;
}

template <typename T>
SharedVariable<T> operator-(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    auto result = make_variable<T>(var1->raw() - var2->raw());
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        Tape<T>::adjoint(var1)->_value += Tape<T>::adjoint(result)->_value;
        Tape<T>::adjoint(var2)->_value -= Tape<T>::adjoint(result)->_value;
    });

    return result;
}

template <typename T>
SharedVariable<T> operator*(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    auto result = make_variable<T>(var1->raw() * var2->raw());
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        Tape<T>::adjoint(var1)->_value += var2->_value * Tape<T>::adjoint(result)->_value;
        Tape<T>::adjoint(var2)->_value += var1->_value * Tape<T>::adjoint(result)->_value;
    });

    return result;
}

template <typename T>
SharedVariable<T> operator/(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    auto result = make_variable<T>(var1->raw() / var2->raw());
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        auto bv2 = var2->_value * var2->_value;

        Tape<T>::adjoint(var1)->_value += var2->_value * Tape<T>::adjoint(result)->_value / bv2;
        Tape<T>::adjoint(var2)->_value -= var1->_value * Tape<T>::adjoint(result)->_value / bv2;
    });

    return result;
}

template <typename T>
SharedVariable<T> make_variable(T value, bool push) 
{
    static_assert(is_same_type<T, float>::value || is_same_type<T, double>::value, "Only double and float");

    struct make_shared_enabler : public Variable<T> 
    { 
    public:
        make_shared_enabler(T val) :
            Variable<T>(val)
        {}
    };

    auto variable = std::make_shared<make_shared_enabler>(value);
    if (push)
    {
        Tape<T>::addToActive(variable);
    }

    return variable;
}


template class Variable<float>;
template class Variable<double>;

template SharedVariable<float> operator+(const SharedVariable<float>& var1, const SharedVariable<float>& var2);
template SharedVariable<double> operator+(const SharedVariable<double>& var1, const SharedVariable<double>& var2);

template SharedVariable<float> operator-(const SharedVariable<float>& var1, const SharedVariable<float>& var2);
template SharedVariable<double> operator-(const SharedVariable<double>& var1, const SharedVariable<double>& var2);

template SharedVariable<float> operator*(const SharedVariable<float>& var1, const SharedVariable<float>& var2);
template SharedVariable<double> operator*(const SharedVariable<double>& var1, const SharedVariable<double>& var2);

template SharedVariable<float> operator/(const SharedVariable<float>& var1, const SharedVariable<float>& var2);
template SharedVariable<double> operator/(const SharedVariable<double>& var1, const SharedVariable<double>& var2);

template SharedVariable<float> make_variable(float value, bool push);
template SharedVariable<double> make_variable(double value, bool push);
