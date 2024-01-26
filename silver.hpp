#pragma once

#include "parse/Ast.hpp"
#include "lib.hpp"

void complain(const char* message, u32 ln);

struct LexCtx
{
	u32 curLn;
};

