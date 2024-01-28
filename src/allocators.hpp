#pragma once

// A collection of allocators designed to manage data lifetimes and allow for fast allocation.
// The general heap is of course very slow because it tries to accomodate for every possibility.
// via custom allocators we can gain big perf as we have prior knowledge about our access patterns.
#include "lib.hpp"

// A hash-table based string uniquer.
// Relies on the stb string hash table impl. Should be pretty good.

struct Str
{
    char* key;
    u16 value;
};
class StringHeap
{
public:
    void initialise();
    void dump();
    
    // Unique a non-null terminated string
    const char* get(const char* src, u32 width);
    
    // Unique a null-terminated string
    //Entry* get(const char* src);

private:
    Str* table = nullptr;
};

// A semi-contiguous no-freeing block allocator.
// This is not synchronised and should only be used by 1 thread.
constexpr u64 BLOCK_ALLOCATOR_BLOCK_SIZE = 1 << 20;
class BlockAllocator
{
private:
    struct Block
    {
        byte data[BLOCK_ALLOCATOR_BLOCK_SIZE];
        u64 size = 0;
        u64 occupancy = 0;

        Block* prev = nullptr;
    };
public:
    void initialise();
    void dump();

    template <typename T>
    T* alloc()
    {
        byte* loc = tail->data + tail->occupancy + alignof(T) - (tail->occupancy % alignof(T));
        if (loc + sizeof(T) >= tail->data + BLOCK_ALLOCATOR_BLOCK_SIZE)
        {
            Block* nb = new Block;
            nb->prev = tail;
            tail = nb;
            loc = tail->data + tail->occupancy + alignof(T) - (tail->occupancy % alignof(T));
        }
        tail->occupancy += sizeof(T);
        new (loc) T{};
        return reinterpret_cast<T*>(loc);
    }

private:
    Block* tail;
};
