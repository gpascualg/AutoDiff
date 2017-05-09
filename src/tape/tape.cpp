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
        tape->_references.emplace(tape->_refcount, variable);
        ++tape->_refcount;
        return true;
    }

    return false;
}


template class Tape<float>;
template class Tape<double>;
