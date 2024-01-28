#pragma once

#include "ast.hpp"

/*
 * Walks the AST, cascading and inferring types across nodes.
 * Also resolves identifiers.
 */
void checkTypes(ParseCtx& ctx);