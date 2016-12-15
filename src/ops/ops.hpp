#pragma once

#include "tape.hpp"


template <typename DType>
class Ops
{
protected:
    Ops()
    {}

public:
    virtual DType* add(Tape* tape, const Shape& shape, DType* a, DType* b) = 0;
    virtual DType* sub(Tape* tape, const Shape& shape, DType* a, DType* b) = 0;
    virtual DType* elementwise_mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) = 0;
    virtual DType* elementwise_div(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) = 0;
    virtual DType* sum(Tape* tape, const Shape& shape, DType* a) = 0;
    virtual DType* sqrt(Tape* tape, const Shape& shape, DType* a) = 0;
    virtual DType* pow(Tape* tape, const Shape& shape, DType* a, float expo) = 0;
    virtual DType* mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) = 0;
    virtual DType* transpose(Tape* tape, const Shape& shape, DType* a) = 0;
};
