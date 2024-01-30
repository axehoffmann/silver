#pragma once

#include <numeric>

#include <vector>
template <typename T>
using Vector = std::vector<T>;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using byte = std::byte;

template <typename T>
class Array
{
public:
    Array(u32 sz) :
        count(sz),
        data(new T[sz]) {}
    
    Array(u32 sz, T* source) :
        count(sz),
        data(new T[sz])
    {
        std::memcpy(data, source, sizeof(T) * sz);
    }

    ~Array()
    {
        delete[] data;
    }

    Array(Array&& o) noexcept :
        count(o.count),
        data(o.data)
    {
        o.data = nullptr;
    }

    Array& operator=(Array&& o)
    {
        data = o.data;
        count = o.count;
        o.data = nullptr;
        return *this;
    }

    T* begin() const
    {
        return data;
    }

    T* end() const
    {
        return data + count;
    }

    T& operator[](u64 i) const
    {
        if (i >= count)
            exit(-1);

        return *(data + i);
    }

    u32 size() const
    {
        return count;
    }
private:
    u32 count;
    T* data;
};

/*
template <typename T>
consteval u32 maxSize()
{
    return sizeof(T);
}

template <typename T, typename ... Ts>
consteval u32 maxSize()
{
    return max(sizeof(T), maxSize<Ts>());
}


template <typename ... Ts>
struct typelist
{
    consteval static maxSize()
    {
        return maxSize<Ts...>();
    }
};
*/