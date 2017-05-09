#include "tape/tape.hpp"
#include "variable/variable.hpp"


template <typename T>
Tape<T>* Tape<T>::_current = nullptr;


template <typename T>
Tape<T>::~Tape()
{
    if (_current = this)
    {
        _current = nullptr;
    }
}

template <typename T>
Tape<T>::Tape()
{}

template <typename T>
void Tape<T>::setActive()
{
    _current = this;
}

template <typename T>
Tape<T>* Tape<T>::active()
{
    return _current;
}

template <typename T>
bool Tape<T>::addToActive(SharedVariable<T> variable)
{
    auto tape = active();
    if (tape)
    {
        tape->_references.emplace(variable, tape->_refcount);
        ++tape->_refcount;
        return true;
    }

    return false;
}

template <typename T>
bool Tape<T>::addOperation(SharedVariable<T> result, SharedVariable<T> operand, Operation&& op)
{
    auto tape = active();
    if (tape)
    {
        tape->addEdge(operand, result);
        tape->addOperation(result, std::move(op));
        return true;
    }

    return false;
}

template <typename T>
bool Tape<T>::addOperation(SharedVariable<T> result, SharedVariable<T> operand1, SharedVariable<T> operand2, Operation&& op)
{
    auto tape = active();
    if (tape)
    {
        tape->addEdge(operand1, result);
        tape->addEdge(operand2, result);
        tape->addOperation(result, std::move(op));
        return true;
    }

    return false;
}

template <typename T>
void Tape<T>::addEdge(SharedVariable<T> from, SharedVariable<T> to)
{
    auto ref1 = _references[from];
    auto ref2 = _references[to];

    auto it = _references.find(from);
    if (it == _references.end())
    {
        _edges[ref1] = std::vector<uint32_t>();
    }

    _edges[ref1].emplace_back(ref2);
}

template <typename T>
void Tape<T>::addOperation(SharedVariable<T> result, Operation&& op)
{
    auto ref = _references[result];

    auto it = _operations.find(ref);
    if (it == _operations.end())
    {
        _operations[ref] = std::vector<Operation>();
    }

    _operations[ref].emplace_back(std::move(op));
}


template class Tape<float>;
template class Tape<double>;
