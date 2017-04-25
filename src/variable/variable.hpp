#pragma once

#include <cmath>
#include <memory>


template <typename T>
inline bool are_close(T a, T b, T rtol=T(1e-5), T atol=T(1e-8))
{
    return std::abs(a - b) <= (atol + rtol * std::abs(b));
}

template <typename T, typename T2>
inline bool are_close(T a, T2 b, T rtol=T(1e-5), T atol=T(1e-8))
{
    return are_close(a, static_cast<T>(b));
}


template <typename T>
class Variable;

template <typename T>
using SharedVariable = std::shared_ptr<Variable<T>>;


template <typename T>
class Variable
{
public:
    static SharedVariable<T> make(T value);

    inline const T& raw() const { return _value; }

    template <typename D> friend SharedVariable<D> operator+(const SharedVariable<D>& var1, const SharedVariable<D>& var2);
    template <typename D> friend SharedVariable<D> operator-(const SharedVariable<D>& var1, const SharedVariable<D>& var2);

protected:
    Variable(T value);

private:
    T _value;
};


template <typename T>
SharedVariable<T> Variable<T>::make(T value) 
{ 
    struct make_shared_enabler : public Variable<T> 
    { 
    public:
        make_shared_enabler(T val) :
            Variable<T>(val)
        {}
    };

    return std::make_shared<make_shared_enabler>(value); 
}
