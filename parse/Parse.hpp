#pragma once
#include "../iter.hpp"
#include "../lex/Token.hpp"
#include "../silver.hpp"

// Parses the token stream to AST until a reasonable stopping point
// ie. a top-level function or other declaration is complete
bool parseAst(Iter<Token>& iter, ParseCtx& ctx);