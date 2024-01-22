#pragma once

#include <numeric>

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

// Lightweight string, with small max size;
class String
{
public:
	String() :
		loc(nullptr),
		sz(0) {}

	String(const char* l, u16 w) :
		loc(new char[w + 1]),
		sz(w + 1)
	{
		std::memcpy(loc, l, w);
		loc[w] = '\0';
	}

	String(String& o) :
		loc(new char[o.sz]),
		sz(o.sz)
	{
		std::memcpy(loc, o.loc, sz);
	}

	String(String&& o) noexcept :
		loc(o.loc),
		sz(o.sz)
	{
		o.loc = nullptr;
	}

	~String()
	{
		delete[] loc;
	}

	const char* data() const
	{
		return loc;
	}

	u16 size() const
	{
		return sz;
	}

	bool operator==(const String& o) const
	{
		return std::strcmp(loc, o.loc);
	}

private:
	char* loc;
	u16 sz;
};

template <typename T>
class Array
{
public:
	Array(u64 sz) :
		count(sz),
		data(new T[sz]) {}

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

private:
	u64 count;
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