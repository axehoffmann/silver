#pragma once

#include "lib.hpp"

class CharIter
{
public:
	const char* begin;
	u64 count;

	CharIter(const char* s, u64 c) :
		begin(s),
		count(c) {}

	char peek(u32 ahead) const
	{
		return *(begin + count + ahead);
	}

	const char* data() const
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