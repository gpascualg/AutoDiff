#include "optimizer.hpp"
#include "tape.hpp"


template <typename DType>
Optimizer<DType>::Optimizer(std::shared_ptr<Bare::Variable<DType>> target, DType learningRate):
    _target(target),
    _learningRate(learningRate)
{}


template <typename DType>
class AdjointsWrapper: public Bare::Variable<DType>
{
public:
    AdjointsWrapper(Bare::SharedVariable<DType> var)
    {
        this->_shape = var->shape();
        this->_values = var->adjoints();
        this->_adjs = nullptr;
    }
};

template <typename DType>
void Optimizer<DType>::reset(float to)
{
    _target->reset(to);

    float learningRate = _learningRate;
    Tape::current()->prepend([learningRate]() {
        for (auto& tapeVariable : Tape::current()->variables())
        {
            auto variable = std::dynamic_pointer_cast<Bare::Variable<DType>>(tapeVariable);
            auto adjoints = std::dynamic_pointer_cast<Bare::Variable<DType>>(std::make_shared<AdjointsWrapper<DType>>(tapeVariable));
            adjoints = variable->_elementwise_mul(adjoints, std::make_shared<Bare::Constant<DType>>(learningRate));
            variable->_sub(variable, adjoints);
        }
    });
}
