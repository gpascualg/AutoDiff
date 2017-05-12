#pragma once

#include <algorithm>
#include <atomic>
#include <functional>
#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "helpers/forward.hpp"
#include "helpers/type_traits.hpp"


struct Node
{
    Node(uint32_t orig);

    uint32_t orig;
    void operator()();

    std::vector<Node*> from;
    std::vector<Node*> to;

    std::vector<Operation> ops;
};


template <typename T>
class Tape
{
public:
    virtual ~Tape();
    
    void setActive();
    void execute(std::initializer_list<SharedVariable<T>> vars);


    static Tape<T>* active();
    static bool addToActive(SharedVariable<T> variable);
    static bool addOperation(SharedVariable<T> result, SharedVariable<T> operand, Operation&& op);
    static bool addOperation(SharedVariable<T> result, SharedVariable<T> operand1, SharedVariable<T> operand2, Operation&& op);
    static SharedVariable<T> adjoint(SharedVariable<T> variable);

    inline std::size_t numVariables() 
    { 
        return _references.size();
    }
    
    inline std::size_t numEdges() 
    { 
        size_t acc = 0;
        std::for_each(_nodes.begin(), _nodes.end(), [&acc](auto p) -> size_t { acc += p.second->to.size(); }); 
        return acc;
    }

    inline std::size_t numOperations() 
    { 
        size_t acc = 0;
        std::for_each(_nodes.begin(), _nodes.end(), [&acc](auto p) -> size_t { acc += p.second->ops.size(); }); 
        return acc;
    }

protected:
    Tape();

private:
    void recurseEdge(Node* node);

    Node* createOrGetNode(SharedVariable<T> variable);
    Node* createOrGetNode(SharedVariable<T> variable, Operation&& op);
    
private:
    std::atomic<uint32_t> _refcount;
    std::unordered_map<SharedVariable<T>, uint32_t> _references;
    std::unordered_map<uint32_t, SharedVariable<T>> _adjoints;
    std::unordered_map<uint32_t, Node*> _nodes;
    Node* _root;

    static Tape<T>* _current;
};


template <typename T>
std::unique_ptr<Tape<T>> make_tape()
{
    static_assert(is_same_type<T, float>::value || is_same_type<T, double>::value, "Only double and float");

    struct make_shared_enabler : public Tape<T> 
    { 
    public:
        make_shared_enabler() :
            Tape<T>()
        {}
    };

    auto tape = std::make_unique<make_shared_enabler>(); 
    tape->setActive();
    return tape;
}
