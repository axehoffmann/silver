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

void printVarExpr(const AstVarExpr& expr)
{
	std::cout << "vl<" << expr.identifier.data() << ">";
}

void printInteger(const AstInteger& i)
{
	std::cout << i.value;
}

void printExpr(const NodePtr& expr)
{
	switch (expr.type)
	{
	case NodeType::VarExpr:
		printVarExpr(*static_cast<const AstVarExpr*>(expr.data));
		return;
	case NodeType::Integer:
		printInteger(*static_cast<const AstInteger*>(expr.data));
		return;
	}
}

void printDecl(const AstDecl& decl)
{
	std::cout << "<decl: " << decl.identifier.data() << ": " << decl.type.data();
	if (decl.valueExpr.data != nullptr)
	{
		std::cout << " = ";
		printExpr(decl.valueExpr);
	}
	std::cout << ">";
}

void printAssign(const AstAssign& asgn)
{
	std::cout << "<assign: ";
	printVarExpr(asgn.lhs);
	std::cout << " = ";
	printExpr(asgn.expr);
	std::cout << ">";
}

void printStatement(const NodePtr& stmt, i32 i)
{
	switch (stmt.type)
	{
	case NodeType::Declaration:
		printDecl(*static_cast<const AstDecl*>(stmt.data));
		return;
	case NodeType::Assignment:
		printAssign(*static_cast<const AstAssign*>(stmt.data));
		return;
	}

	std::cout << "don't know how to print this statement";
}

void printBlock(const AstBlock& b, i32 i)
{
	indent(i); std::cout << "{\n";

	for (const NodePtr& node : b.statements)
	{
		indent(i + 1); printStatement(node, i + 1); std::cout << "\n";
	}

	indent(i); std::cout << "}\n";
}

void printFn(const AstFn& fn)
{
	std::cout << "<func: " << fn.name.data() << ", params(";
	for (const AstDecl& decl : fn.parameters)
	{
		std::cout << decl.identifier.data() << ": " << decl.type.data() << ",";
	}
	std::cout << ")> \n";

	printBlock(fn.block, 0);
}

