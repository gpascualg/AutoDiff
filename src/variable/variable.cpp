#include "variable/variable.hpp"


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
