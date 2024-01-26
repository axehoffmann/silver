#pragma once

#include "../parse/Ast.hpp"

/*
 * Walks the AST, cascading and inferring types across nodes.
 */
void checkTypes(ParseCtx& ctx);