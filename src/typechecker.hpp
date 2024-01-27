#pragma once

#include "ast.hpp"

/*
 * Walks the AST, cascading and inferring types across nodes.
 */
void checkTypes(ParseCtx& ctx);