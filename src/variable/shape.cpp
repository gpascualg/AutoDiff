#include "variable/shape.hpp"

#include <algorithm>
#include <assert.h>


template <uint8_t Dims>
Shape<Dims>::Shape(const IShape& shape)
{
    for (int i = 0; i < Dims; ++i)
    {
        if (i < shape.ndims())
        {
            _dims[i] = shape[i];
        }
        else
        {
            _dims[i] = 1;
        }
    }
}

template <uint8_t Dims>
uint8_t Shape<Dims>::ndims() const
{
    return Dims;
}

template <uint8_t Dims>
uint32_t Shape<Dims>::prod() const
{
    return std::accumulate(_dims, _dims + Dims, 1, std::multiplies<uint32_t>());
}

template <uint8_t Dims>
uint32_t Shape<Dims>::operator[](uint8_t idx) const
{
    assert(idx < Dims);
    return _dims[idx];
}

template <uint8_t Dims>
bool Shape<Dims>::operator==(const IShape& other) const
{
    std::size_t nmax = std::max(ndims(), other.ndims());

    for (int i = 0; i < nmax; ++i)
    {
        if (other[i] != _dims[i])
        {
            return false;
        }
    }

    for (int i = Dims; i < nmax; ++i)
    {
        if (_dims[i] != 1)
        {
            return false;
        }
    }

    for (int i = other.ndims(); i < nmax; ++i)
    {
        if (other[i] != 1)
        {
            return false;
        }
    }

    return true;
}


/*
template <uint8_t Dims>
bool Shape<Dims>::operator==(const Shape<Dims>& other) const
{
    return *this == static_cast<IShape>(other);
}
*/


template struct Shape<1>;
template struct Shape<2>;
template struct Shape<3>;
template struct Shape<4>;