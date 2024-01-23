#pragma once

#include "lib.hpp"

template <typename T>
class Iter
{
public:
	T* begin;
	u64 count;

	Iter(T* s) :
		begin(s),
		count(0) {}

	T& peek(u32 ahead) const
	{
		return *(begin + count + ahead);
	}

	T* data() const
	{
		return begin + count;
	}

	u32 position() const
	{
		return count;
	}

	void advance(u32 forward)
	{
		count += forward;
	}

	/*
	bool operator==(const CharIter& o) const
	{
		return begin == o.begin && count == o.count;
	}

	char* begin() const
	{
		return begin;
	}

	char* end() const
	{
		return begin + count;
	}
	*/
};