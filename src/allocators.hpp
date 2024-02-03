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

#include <vector>
#include <iostream>
#include <atomic>

constexpr u64 BLOCK_ARRAY_BLOCK_SIZE = 32;
// A dynamic array type that has pointer stability by allocating in discrete blocks
template <typename T>
class BlockArray
{
private:
    struct Block
    {
        byte data[BLOCK_ARRAY_BLOCK_SIZE * sizeof(T)];
        u64 occupancy;
    };

public:
    struct iterator
    {
        iterator& operator++()
        {
            idx++;
            if (idx >= BLOCK_ARRAY_BLOCK_SIZE)
            {
                idx = 0;
                blocknum++;
            }
            return *this;
        }

        T& operator*() const
        {
            return *reinterpret_cast<T*>(&arr->blocks[blocknum]->data[idx * sizeof(T)]);
        }

        friend bool operator==(const iterator& a, const iterator& b)
        {
            return a.blocknum == b.blocknum && a.idx == b.idx;
        }

        const BlockArray* arr;
        u64 blocknum;
        u64 idx;
    };

    BlockArray()
    {
        blocks.push_back(new Block{});
    }

    ~BlockArray()
    {
        for (Block* b : blocks)
            delete b;
    }
    void push_back(T&& val)
    {
        Block* b = blocks.back();
        if (b->occupancy >= BLOCK_ARRAY_BLOCK_SIZE)
        {
            blocks.push_back(new Block{});
            b = blocks.back();
        }

        *reinterpret_cast<T*>(&b->data[b->occupancy * sizeof(T)]) = std::move(val);
        std::cout << b->occupancy;
        b->occupancy++;
    }

    T& at(u64 i) const
    {
        u64 blocknum = i / BLOCK_ARRAY_BLOCK_SIZE;
        u64 idx = i % BLOCK_ARRAY_BLOCK_SIZE;
        return blocks[blocknum]->data[idx];
    }

    iterator begin() const
    {
        return iterator{ this, 0, 0 };
    }

    iterator end() const
    {
        return iterator{ this, blocks.size() - 1, blocks.back()->occupancy };
    }

    T& back() const
    {
        return *reinterpret_cast<T*>(&blocks.back()->data[(blocks.back()->occupancy - 1) * sizeof(T)]);
    }

private:
    std::vector<Block*> blocks;
};

// A BlockArray that functions correctly in a multithreaded context (in terms of reads and push_backs)
