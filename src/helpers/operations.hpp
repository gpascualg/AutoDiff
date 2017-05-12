#pragma once

#define ARE_CLOSE(x, y) std::abs((x) - (y)) <= (1e-5 + 1e-8 * std::abs((y)))

template <typename T>
inline bool are_close(T a, T b, T rtol=T(1e-5), T atol=T(1e-8))
{
    return std::abs(a - b) <= (atol + rtol * std::abs(b));
}

template <typename T, typename T2>
inline bool are_close(T a, T2 b, T rtol=T(1e-5), T atol=T(1e-8))
{
    return are_close(a, static_cast<T>(b));
}
