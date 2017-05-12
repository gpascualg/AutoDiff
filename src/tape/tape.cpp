#include "tape/tape.hpp"
#include "variable/variable.hpp"


template <typename T>
Tape<T>* Tape<T>::_current = nullptr;

Node::Node(uint32_t orig):
    orig(orig)
{}

void Node::operator()()
{
    for (auto&& op: ops)
    {
        op();
    }
}

template <typename T>
Tape<T>::~Tape()
{
    if (_current = this)
    {
        _current = nullptr;
    }
}

template <typename T>
Tape<T>::Tape():
    _refcount(0),
    _root(nullptr)
{}

template <typename T>
void Tape<T>::setActive()
{
    _current = this;
}

template <typename T>
void Tape<T>::execute(std::initializer_list<SharedVariable<T>> vars)
{
    for (auto var : vars)
    {
        auto ref = _references[var];
        _adjoints[ref]->_value = 1;
    }

    for (auto var : vars)
    {
        auto ref = _references[var];
        recurseEdge(_nodes[ref]);
    }
}

#include <iostream>

template <typename T>
void Tape<T>::recurseEdge(Node* node)
{
    (*node)();

    for (auto from: node->from)
    {
        recurseEdge(from);
    }
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
        tape->_adjoints.emplace(tape->_refcount, make_variable<T>(0, false));
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
        Node* from = tape->createOrGetNode(operand);
        Node* to = tape->createOrGetNode(result, std::move(op));

        to->from.push_back(from);
        from->to.push_back(to);

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
        Node* from1 = tape->createOrGetNode(operand1);
        Node* from2 = tape->createOrGetNode(operand2);
        Node* to = tape->createOrGetNode(result, std::move(op));

        to->from.push_back(from1);
        to->from.push_back(from2);
        from1->to.push_back(to);
        from2->to.push_back(to);
        
        return true;
    }

    return false;
}

template <typename T>
SharedVariable<T> Tape<T>::adjoint(SharedVariable<T> variable)
{
    auto tape = active();
    if (tape)
    {
        auto ref = tape->_references[variable];
        return tape->_adjoints[ref];
    }

    return nullptr;
}

template <typename T>
Node* Tape<T>::createOrGetNode(SharedVariable<T> variable)
{
    auto ref = _references[variable];
    auto it = _nodes.find(ref);
    
    if (it != _nodes.end())
    {
        return it->second;
    }

    Node* node = new Node(ref);
    _nodes.emplace(ref, node);
    return node;
}

template <typename T>
Node* Tape<T>::createOrGetNode(SharedVariable<T> variable, Operation&& op)
{
    auto node = createOrGetNode(variable);
    node->ops.emplace_back(std::move(op));
    return node;
}

template class Tape<float>;
template class Tape<double>;
