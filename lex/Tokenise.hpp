#pragma once

#include "Token.hpp"
#include "../iter.hpp"
#include "../silver.hpp"

// Parses a single token and advances the iterator.
Token parseToken(Iter<const char>& iter, ParseCtx& ctx);

