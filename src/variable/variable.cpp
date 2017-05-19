#include "variable/variable.hpp"
#include "tape/tape.hpp"


template <typename T>
Variable<T>::Variable(T value) :
    _shape(1, 1)
{
    _value = new T;
    *_value = value;
}


template <typename T>
Variable<T>::Variable(Shape<2> shape, T value) :
    _shape(shape)
{
    _value = new T[shape.prod()];

    LOOP(this) { AT(this) = value; }
}

template <typename T>
SharedVariable<T> operator+(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    ASSERT_SAME_SHAPE(var1, var2);

    auto result = make_variable<T>(var1->_shape, 0);
    LOOP(result) { AT(result) = AT(var1) + AT(var2); }
    
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        LOOP(var1) { AT(ADJ(var1)) += AT(ADJ(result)); }
        LOOP(var2) { AT(ADJ(var2)) += AT(ADJ(result)); }
    });

    return result;
}

template <typename T>
SharedVariable<T> operator-(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    ASSERT_SAME_SHAPE(var1, var2);

    auto result = make_variable<T>(var1->_shape, 0);
    LOOP(result) { AT(result) = AT(var1) - AT(var2); }
    
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        LOOP(var1) { AT(ADJ(var1)) += AT(ADJ(result)); }
        LOOP(var2) { AT(ADJ(var2)) -= AT(ADJ(result)); }
    });

    return result;
}

template <typename T>
SharedVariable<T> operator*(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    ASSERT_SAME_SHAPE(var1, var2);

    auto result = make_variable<T>(var1->_shape, 0);
    LOOP(result) { AT(result) = AT(var1) * AT(var2); }
    
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        LOOP(var1) { AT(ADJ(var1)) += AT(var2) * AT(ADJ(result)); }
        LOOP(var2) { AT(ADJ(var2)) += AT(var1) * AT(ADJ(result)); }
    });

    return result;
}

template <typename T>
SharedVariable<T> operator/(const SharedVariable<T>& var1, const SharedVariable<T>& var2)
{
    ASSERT_SAME_SHAPE(var1, var2);

    auto result = make_variable<T>(var1->_shape, 0);
    LOOP(result) { AT(result) = AT(var1) / AT(var2); }
    
    Tape<T>::addOperation(result, var1, var2, [var1, var2, result](){
        LOOP(var1) { AT(ADJ(var1)) += AT(var2) * AT(ADJ(result)) / (AT(var2) * AT(var2)); }
        LOOP(var2) { AT(ADJ(var2)) -= AT(var1) * AT(ADJ(result)) / (AT(var2) * AT(var2)); }
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

template <typename T>
SharedVariable<T> make_variable(Shape<2> shape, T value, bool push) 
{
    static_assert(is_same_type<T, float>::value || is_same_type<T, double>::value, "Only double and float");

    struct make_shared_enabler : public Variable<T> 
    { 
    public:
        make_shared_enabler(Shape<2> shape, T val) :
            Variable<T>(shape, val)
        {}
    };

    auto variable = std::make_shared<make_shared_enabler>(shape, value);
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
