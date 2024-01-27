#pragma once
#include "lexer.hpp"
#include "ast.hpp"

// Parses the token stream to AST until a reasonable stopping point
// ie. a top-level function or other declaration is complete
bool parseAst(Lexer& lx, ParseCtx& ctx);