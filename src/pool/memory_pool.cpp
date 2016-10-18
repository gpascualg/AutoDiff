#include "memory_pool.hpp"

#include <cstring>


Pool* Pool::_instance = nullptr;

Pool::Pool(bool canGrow, uint32_t maxSize):
    _canGrow(canGrow),
    _maxSize(maxSize)
{
    _thread = std::move(std::thread(&Pool::startZeroInitializer, this));
}

void* Pool::create(uint32_t size)
{
    void* memory = malloc(size);
    std::memset(memory, 0, size);
    return memory;
}
