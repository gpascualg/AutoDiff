#include "optimizer.hpp"
#include "tape.hpp"


template <typename DType>
Optimizer<DType>::Optimizer(std::shared_ptr<Bare::Variable<DType>> target, DType learningRate):
    _target(target),
    _learningRate(learningRate)
{}


template <typename DType>
class AdjointsWrapper: public Bare::Constant<DType>
{
public:
    AdjointsWrapper(std::shared_ptr<TapeVariable> var):
        Bare::Constant<DType>(var->shape())
    {
        //this->_shape = var->shape();
        this->_values = (DType*)var->untyped_adjoints();
    }
};

template <typename DType>
void Optimizer<DType>::reset(float to)
{
    _target->reset(to);

    float learningRate = _learningRate;
    //printf("TAPE SIZE=%d, TAPE VARIABLES=%d\n", Tape::current()->size(), Tape::current()->variables().size());
    Tape::current()->prepend([learningRate]() {
        for (auto& tapeVariable : Tape::current()->variables())
        {
            if (tapeVariable->isTrainable())
            {
                auto variable = std::dynamic_pointer_cast<Bare::Variable<DType>>(tapeVariable);
                auto adjoints = std::dynamic_pointer_cast<SpecializedTapeVariable<DType>>(std::make_shared<AdjointsWrapper<DType>>(tapeVariable));
                auto lr = std::dynamic_pointer_cast<SpecializedTapeVariable<DType>>(std::make_shared<Bare::Constant<DType>>(learningRate));
                adjoints = variable->_elementwise_mul(adjoints, lr);
                variable->update(variable->_sub(variable, adjoints));
            }
        }
    });
}

// SPECIALIZATIONS
template class Optimizer<float>;
template class Optimizer<double>;
