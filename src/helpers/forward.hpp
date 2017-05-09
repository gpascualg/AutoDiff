#pragma once


template <typename T>
class Variable;

template <typename T>
using SharedVariable = std::shared_ptr<Variable<T>>;
