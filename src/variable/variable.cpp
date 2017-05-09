#include "variable/variable.hpp"
#include "tape/tape.hpp"


template <typename T>
Variable<T>::Variable(T value) : 
    _value{ value }
{}

template <typename T>
SharedVariable<T> operator+(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    return make_variable<T>(var1->raw() + var2->raw());
}

template <typename T>
SharedVariable<T> operator-(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    return make_variable<T>(var1->raw() - var2->raw());
}

template <typename T>
SharedVariable<T> operator*(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    return make_variable<T>(var1->raw() * var2->raw());
}

template <typename T>
SharedVariable<T> operator/(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    return make_variable<T>(var1->raw() / var2->raw());
}

template <typename T>
SharedVariable<T> make_variable(T value) 
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
    Tape<T>::addToActive(variable);
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

template SharedVariable<float> make_variable(float value);
template SharedVariable<double> make_variable(double value);
