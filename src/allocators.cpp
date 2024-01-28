#include "allocators.hpp"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include <iostream>

void StringHeap::initialise()
{
    sh_new_arena(table);
}

void StringHeap::dump()
{
    shfree(table);
}

constexpr u32 LARGE_STRING_THRESHOLD = 1024;

const char* StringHeap::get(const char* src, u32 width)
{
    // For small strings, use alloca to create a buffer where
    // we can add the null terminator. Use a heap alloc for large strings,
    // and we don't bother uniqueing them.
    char* buffer = nullptr;
    if (width < LARGE_STRING_THRESHOLD)
    {
        buffer = static_cast<char*>(alloca(width + 1));
        memcpy(buffer, src, width);
        buffer[width] = '\0';
        shput(table, buffer, width);
        return (table + shgeti(table, buffer))->key;
    }

    // #TODO: When large strings become a thing to parse,
    // we will likely want a system where we allocate them to a list.
    // Other components can then claim the string out of the list later to avoid a big copy.
    
    // heapalloc = true;
    // buffer = new char[width + 1];
    std::cout << "Can't allocate big strings yet!\n";
    return nullptr;
}

void BlockAllocator::initialise()
{
    tail = new Block;
}

void BlockAllocator::dump()
{
    Block* pos = tail;
    while (pos->prev != nullptr)
    {
        Block* next = pos->prev;
        delete pos;
        pos = next;
    }
}
