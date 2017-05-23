#pragma once

#include <inttypes.h>


struct IShape
{
    virtual uint8_t ndims() const = 0;
    virtual uint32_t prod() const = 0;
    virtual uint32_t operator[](uint8_t idx) const = 0;
    virtual bool operator==(const IShape& other) const = 0;
};

template <uint8_t Dims = 2>
struct Shape : public IShape
{
    template <typename... T>
    Shape(T... d);

    Shape(const IShape& shape);

    uint8_t ndims() const override;
    uint32_t prod() const override;
    uint32_t operator[](uint8_t idx) const override;
    bool operator==(const IShape& other) const override;
    //bool operator==(const Shape<Dims>& other) const;
    
private:
    uint32_t _dims[Dims];
};


template <uint8_t Dims>
template <typename... T>
Shape<Dims>::Shape(T... d)
{
    static_assert(sizeof...(d) == Dims, "Wrong dimensions");

    int i = 0;
    for (auto& v: {d...})
    {
        _dims[i++] = v;
    }
}
