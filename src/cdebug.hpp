#pragma once
#include "ast.hpp"
#include "token.hpp"

struct TypeRef;

// Functions for debugging the compiler

void printExpr(const NodePtr& expr);
void printFn(const AstFn& fn);
void printFnIface(const AstFnInterface& fn);
void printType(const TypeRef& ty)
