#pragma once

#include <atomic>
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

    inline std::size_t numVariables() { return _references.size(); }

protected:
    Tape();

private:
    std::atomic<uint32_t> _refcount;
    std::unordered_map<uint32_t, SharedVariable<T>> _references;
    std::unordered_map<uint32_t, uint32_t> _edges;

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
