#pragma once

#include "parse/Ast.hpp"
#include "lib.hpp"

struct ParseCtx
{
	Vector<AstFn> functions;
	Vector<AstDecl> variables;

	std::byte buffer[1 << 28];
	u64 bptr = 0;
};

template <typename T>
T* allocate(ParseCtx& ctx)
{
	T* loc = reinterpret_cast<T*>(&(ctx.buffer[ctx.bptr]));
	ctx.bptr += sizeof(T);
	std::construct_at(loc);
	return loc;
}
