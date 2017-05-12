#pragma once

#include <cmath>
#include <memory>

#include "helpers/forward.hpp"
#include "helpers/type_traits.hpp"


template <typename T>
class Variable
{
    template <typename D> friend class Tape;
    
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


template <typename T> SharedVariable<T> make_variable(T value, bool push = true);
