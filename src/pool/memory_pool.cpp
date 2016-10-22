#include "memory_pool.hpp"

#include <cstring>


Pool::Pool(bool canGrow, uint32_t maxSize):
    _canGrow(canGrow),
    _maxSize(maxSize),
    _stop(false)
{
    _thread = std::move(std::thread(&Pool::startZeroInitializer, this));
}

void* Pool::create(uint32_t size)
{
    void* memory = malloc(size);
    std::memset(memory, 0, size);
    return memory;
}

void Pool::release()
{
    _stop = true;
    _workSignal.notify_all();
    _thread.join();

    while (!_freeBlocks.empty())
    {
        free(_freeBlocks.back().memory);
        _freeBlocks.pop_back();
    }

    while (!_pendingBlocks.empty())
    {
        free(_pendingBlocks.back().memory);
        _pendingBlocks.pop_back();
    }
}
