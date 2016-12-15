#pragma once

#include "tape.hpp"
#include "ops.hpp"

template <typename DType>
class CPUOps : public Ops<DType>
{
public:
    using Ops<DType>::Ops;

    static CPUOps<DType>* get() {
        if (!_instance)
        {
            _instance = new CPUOps();
        }

        return _instance;
    }

    DType* add(Tape* tape, const Shape& shape, DType* a, DType* b) override;
    DType* sub(Tape* tape, const Shape& shape, DType* a, DType* b) override;
    DType* elementwise_mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) override;
    DType* elementwise_div(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) override;
    DType* sum(Tape* tape, const Shape& shape, DType* a) override;
    DType* sqrt(Tape* tape, const Shape& shape, DType* a) override;
    DType* pow(Tape* tape, const Shape& shape, DType* a, float expo) override;
    DType* mul(Tape* tape, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b) override;
    DType* transpose(Tape* tape, const Shape& shape, DType* a) override;

private:
    static CPUOps<DType>* _instance;
};
