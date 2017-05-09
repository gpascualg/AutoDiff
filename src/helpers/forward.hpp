#pragma once

#include <functional>


template <typename T>
class Variable;

template <typename T>
using SharedVariable = std::shared_ptr<Variable<T>>;

using Operation = std::function<void(void)>;