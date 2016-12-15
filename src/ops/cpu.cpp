#include "ops/cpu.hpp"
#include "pool/memory_pool.hpp"

#include <cmath>


template <typename DType>
CPUOps<DType>* CPUOps<DType>::_instance = nullptr;


template <typename DType>
DType* CPUOps<DType>::add(Tape* tape, const Shape& shape, DType* a, DType* b)
{
    auto r = tape->pool()->allocate<DType>(shape.prod());

    SHAPE_LOOP(shape)
    {
        r[CIDX(shape)] = a[CIDX(shape)] + b[CIDX(shape)];
    }

    tape->add(r, shape, a, b);

    return r;
}

template <typename DType>
DType* CPUOps<DType>::sub(Tape* tape, const Shape& shape, DType* a, DType* b)
{
    auto r = tape->pool()->allocate<DType>(shape.prod());

    SHAPE_LOOP(shape)
    {
        r[CIDX(shape)] = a[CIDX(shape)] - b[CIDX(shape)];
    }

    return r;
}

template <typename DType>
DType* CPUOps<DType>::elementwise_mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
    if (shape_a.isUnitary())
    {
        if (!shape_b.isUnitary())
        {
            // TODO(gpascualg): ERROR
        }

        a[0] *= b[0];
    }
    else if (shape_b.isUnitary())
    {
        SHAPE_LOOP(shape_a)
        {
            a[CIDX(shape_a)] *= b[0];
        }

        return a;
    }
    else
    {
        auto r = tape->pool()->allocate<DType>(shape_a.prod());

        SHAPE_LOOP(shape_a)
        {
            r[CIDX(shape_a)] = a[CIDX(shape_a)] * b[CIDX(shape_a)];
        }

        return r;
    }
}

template <typename DType>
DType* CPUOps<DType>::elementwise_div(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
    if (shape_a.isUnitary())
    {
        if (!shape_b.isUnitary())
        {
            // TODO(gpascualg): ERROR
        }

        a[0] /= b[0];
    }
    else if (shape_b.isUnitary())
    {
        SHAPE_LOOP(shape_a)
        {
            a[CIDX(shape_a)] /= b[0];
        }

        return a;
    }
    else
    {
        auto r = tape->pool()->allocate<DType>(shape_a.prod());

        SHAPE_LOOP(shape_a)
        {
            r[CIDX(shape_a)] = a[CIDX(shape_a)] / b[CIDX(shape_a)];
        }

        return r;
    }
}

template <typename DType>
DType* CPUOps<DType>::sum(Tape* tape, const Shape& shape, DType* a)
{
    auto r = tape->pool()->allocate<DType>(1);

    SHAPE_LOOP(shape) {
        r[0] += a[CIDX(shape)];
    }

    return r;
}

template <typename DType>
DType* CPUOps<DType>::sqrt(Tape* tape, const Shape& shape, DType* a)
{
    auto r = tape->pool()->allocate<DType>(shape.prod());

    SHAPE_LOOP(shape) {
        r[CIDX(shape)] = std::sqrt(a[CIDX(shape)]);
    }

    return r;
}

template <typename DType>
DType* CPUOps<DType>::pow(Tape* tape, const Shape& shape, DType* a, float expo)
{
    auto r = tape->pool()->allocate<DType>(shape.prod());

    SHAPE_LOOP(shape) {
        r[CIDX(shape)] = std::pow(a[CIDX(shape)], expo);
    }

    return r;
}

template <typename DType>
DType* CPUOps<DType>::mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
    Shape shape_r = { shape_a.m, shape_b.n };
    auto r = tape->pool()->allocate<DType>(shape_r.prod());

    for (int i = 0; i < shape_a.m; ++i)
    {
        for (int j = 0; j < shape_b.n; ++j)
        {
            for (int k = 0; k < shape_a.n; ++k)
            {
                r[CIDX_(shape_r, i, j)] += a[CIDX_(shape_a, i, k)] * b[CIDX_(shape_b, k, j)];
            }
        }
    }

    return r;
}

template <typename DType>
DType* CPUOps<DType>::transpose(Tape* tape, const Shape& shape, DType* a)
{
    auto r = tape->pool()->allocate<DType>(shape.T().prod());

    SHAPE_LOOP(shape) {
        r[CIDX_(shape, y, x)] = a[CIDX(shape)];
    }

    return r;
}



// SPECIALIZE
template class CPUOps<float>;
template class CPUOps<double>;
