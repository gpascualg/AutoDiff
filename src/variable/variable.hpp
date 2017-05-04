#pragma once

#include <cmath>
#include <memory>

#include "helpers/type_traits.hpp"


template <typename T>
class Variable;

template <typename T>
using SharedVariable = std::shared_ptr<Variable<T>>;


template <typename T>
class Variable
{
public:
    inline const T& raw() const { return _value; }

    template <typename D> friend SharedVariable<D> operator+(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator-(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator*(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator/(const SharedVariable<D>& var1, const SharedVariable<D>& var2);

protected:
    Variable(T value);

private:
    T _value;
};


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

    return std::make_shared<make_shared_enabler>(value); 
}
