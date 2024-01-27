#pragma once
#include "ast.hpp"
#include "token.hpp"

// Functions for debugging the compiler


void printFn(const AstFn& fn);
void printFnIface(const AstFnInterface& fn);