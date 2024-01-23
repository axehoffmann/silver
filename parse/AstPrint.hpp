#pragma once

#include "Ast.hpp"
#include <iostream>

void indent(i32 tabs)
{
	for (i32 i = 0; i < tabs; i++)
	{
		std::cout << '\t';
	}
}

void printExpr(const NodePtr& expr)
{
	switch (expr.type)
	{
	case NodeType::VarExpr:
		std::cout << (static_cast<const AstVarExpr*>(expr.data))->identifier.data();
		return;
	}
}

void printDecl(const AstDecl& decl)
{
	std::cout << decl.identifier.data() << ": " << decl.type.data();
	if (decl.valueExpr.data != nullptr)
	{
		std::cout << " = ";
		printExpr(decl.valueExpr);
	}
}

void printStatement(const NodePtr& stmt, i32 i)
{
	switch (stmt.type)
	{
	case NodeType::Declaration:
		printDecl(*static_cast<const AstDecl*>(stmt.data));
		return;
	}
}

void printBlock(const AstBlock& b, i32 i)
{
	indent(i); std::cout << "{\n";

	for (const NodePtr& node : b.statements)
	{
		indent(i + 1); printStatement(node, i + 1);
	}

	indent(i); std::cout << "}\n";
}

void printFn(const AstFn& fn)
{
	std::cout << "Function: " << fn.name.data() << ": fn(";
	for (const AstDecl& decl : fn.parameters)
	{
		std::cout << decl.identifier.data() << ": " << decl.type.data() << ",";
	}
	std::cout << ") \n";

	printBlock(fn.block, 0);
}

