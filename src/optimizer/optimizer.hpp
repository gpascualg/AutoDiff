#pragma once

#include "variable.hpp" // TODO(gpascualg): Forward declare?
#include "tape.hpp"

template <typename DType>
class Optimizer : public TapeVariable
{
public:
    Optimizer(std::shared_ptr<Bare::Variable<DType>> target, DType learningRate);

    void reset(float to) override;

    inline void* untyped_values() override { return nullptr; }
    inline void* untyped_adjoints() override { return nullptr; }

private:
    std::shared_ptr<Bare::Variable<DType>> _target;
    DType _learningRate;
};
