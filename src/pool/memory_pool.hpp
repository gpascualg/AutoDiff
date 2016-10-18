#pragma once

#include <inttypes.h>
#include <condition_variable>
#include <queue>
#include <list>
#include <mutex>
#include <thread>



struct MemoryBlock
{
    MemoryBlock(uint32_t s, void* m):
        size(s), memory(m)
    {}

    uint32_t size;
    void* memory;
};

struct OrderDescending
{
    bool operator()(const MemoryBlock& a, const MemoryBlock&b)
    {
        return a.size > b.size;
    }
};

struct OrderAscending
{
    bool operator()(const MemoryBlock& a, const MemoryBlock&b)
    {
        return a.size < b.size;
    }
};

class Pool
{
public:
    static Pool* get()
    {
        if (!_instance)
        {
            _instance = new Pool(true, 1);
        }

        return _instance;
    }

    template <typename T>
    void* allocate(uint32_t size);

    template <typename T>
    void deallocate(void* memory, uint32_t size);

private:
    Pool(bool canGrow, uint32_t maxSize);

    static void startZeroInitializer(Pool* pool)
    {
        while (true)
        {
            // Wait for work
            std::unique_lock<std::mutex> workLock(pool->_work);
            pool->_workSignal.wait(workLock);

            // Do work
            while (!pool->_pendingBlocks.empty())
            {
                pool->_lockPendingQueue.lock();

                MemoryBlock block = pool->_pendingBlocks.back();
                pool->_pendingBlocks.pop_back();
                std::pop_heap(pool->_pendingBlocks.begin(), pool->_pendingBlocks.end(), OrderDescending());

                pool->_lockPendingQueue.unlock();

                uint32_t size = block.size;
                void* memory = block.memory;

                for (int i = 0; i < size; ++i)
                {
                    *(char*)((uintptr_t)memory + i) = 0;
                }

                pool->_lockFreeQueue.lock();
                pool->_freeBlocks.push_back(block);
                std::push_heap(pool->_freeBlocks.begin(), pool->_freeBlocks.end(), OrderAscending());
                pool->_lockFreeQueue.unlock();
            }
        }
    }

    void* create(uint32_t size);

private:
    static Pool* _instance;
    std::thread _thread;

    bool _canGrow;
    uint32_t _maxSize;

    std::mutex _work;
    std::condition_variable _workSignal;

    std::mutex _lockFreeQueue;
    std::mutex _lockPendingQueue;

    std::vector<MemoryBlock> _pendingBlocks;
    std::vector<MemoryBlock> _freeBlocks;
};


template <typename T>
void* Pool::allocate(uint32_t size)
{
    uint32_t memsize = size * sizeof(T);
    std::lock_guard<std::mutex> lock(_lockFreeQueue);

    if (_freeBlocks.empty())
    {
        return create(memsize);
    }

    for (auto it = _freeBlocks.cbegin(); it != _freeBlocks.cend(); ++it)
    {
        if ((*it).size >= memsize)
        {
            void* memory = (*it).memory;
            _freeBlocks.erase(it);
            return memory;
        }
    }

    // No block big enough has been found
    return create(memsize);
}

template <typename T>
void Pool::deallocate(void* memory, uint32_t size)
{
    std::lock_guard<std::mutex> lock(_lockPendingQueue);

    _pendingBlocks.emplace_back(size * sizeof(T), memory);
    std::push_heap(_pendingBlocks.begin(), _pendingBlocks.end(), OrderDescending());

    _workSignal.notify_one();
}
