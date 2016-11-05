#include "optimizer.hpp"
#include "tape.hpp"


template <typename DType>
Optimizer<DType>::Optimizer(std::shared_ptr<Bare::Variable<DType>> target, DType learningRate):
    _target(target),
    _learningRate(learningRate)
{}

template <typename DType>
void Optimizer<DType>::reset(float to)
{
    _target->reset(to);

    Tape::current()->prepend([]() {
        for (auto& tapeVariable : Tape::current()->variables())
        {
            auto variable = std::dynamic_pointer_cast<Bare::Variable<DType>>(tapeVariable);
        }
    });
}
