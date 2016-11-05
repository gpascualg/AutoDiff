#pragma once

#include "variable.hpp" // TODO(gpascualg): Forward declare?


template <typename DType>
class Optimizer
{
public:
    explicit Optimizer(std::shared_ptr<Bare::Variable<DType>> target, DType learningRate);

    void reset(float to) override;

private:
    std::shared_ptr<Bare::Variable<DType>> _target;
    DType _learningRate;
};

template <> class Optimizer<float>;
template <> class Optimizer<double>;
