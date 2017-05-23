#pragma once

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <memory>
#include <inttypes.h>

#include "helpers/forward.hpp"
#include "helpers/type_traits.hpp"
#include "variable/shape.hpp"


#define ASSERT_SAME_SHAPE(var1, var2) assert(var1->_shape == var2->_shape)
#define LOOP(var) for (int i = 0; i < var->_shape.prod(); ++i)
#define ADJ(var) Tape<T>::adjoint(var)
#define AT(var) var->_value[i]


template <typename T>
class Variable
{
    template <typename D> friend class Tape;
    
public:
    inline const T& raw() const 
    { 
        assert(_shape == Shape<2>({1, 1}));
        return *_value; 
    }

    inline const T& raw(uint32_t i) const
    {
        assert(i < _shape.prod());
        return _value[i];
    }

    inline const T& raw(uint32_t x, uint32_t y) const
    {
        assert(x < _shape[0] && y < _shape[1]);
        return _value[y * _shape[0] + x];
    }

    template <typename D> friend SharedVariable<D> operator+(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator-(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator*(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator/(const SharedVariable<D>& var1, const SharedVariable<D>& var2);

protected:
    Variable(T value);
    Variable(Shape<2> shape, T value);

private:
    T* _value;
    Shape<2> _shape;
};


template <typename T> SharedVariable<T> make_variable(T value, bool push = true);
template <typename T> SharedVariable<T> make_variable(Shape<2> shape, T value, bool push = true);
