#pragma once

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "helpers/forward.hpp"
#include "helpers/type_traits.hpp"


template <typename T>
class Tape
{
public:
    virtual ~Tape();
    void setActive();
    static Tape<T>* active();

    static bool addToActive(SharedVariable<T> variable);
    static bool addOperation(SharedVariable<T> result, SharedVariable<T> operand, Operation&& op);
    static bool addOperation(SharedVariable<T> result, SharedVariable<T> operand1, SharedVariable<T> operand2, Operation&& op);

    inline std::size_t numVariables() { return _references.size(); }
    inline std::size_t numEdges() 
    { 
        size_t acc = 0;
        std::for_each(_edges.begin(), _edges.end(), [&acc](auto p) -> size_t { acc += p.second.size(); }); 
        return acc;
    }

protected:
    Tape();

private:
    void addEdge(SharedVariable<T> from, SharedVariable<T> to);

private:
    std::atomic<uint32_t> _refcount;
    std::unordered_map<SharedVariable<T>, uint32_t> _references;
    std::unordered_map<uint32_t, std::vector<uint32_t>> _edges;
    std::unordered_map<uint32_t, Operation> _operations;

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
