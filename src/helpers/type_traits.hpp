#pragma once

template <class A, class B>
struct is_same_type
{
    static const bool value = false;
};

template <class A>
struct is_same_type<A, A>
{
    static const bool value = true;
};
