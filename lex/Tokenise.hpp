#pragma once

#include "Token.hpp"
#include "../iter.hpp"

class ParseCtx
{

};

// Parses a single token and advances the iterator.
Token parseToken(CharIter& iter, ParseCtx& ctx);

