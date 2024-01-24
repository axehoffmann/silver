#pragma once

#include "../lex/Token.hpp"
#include "../lib.hpp"
#include "../silver.hpp"

enum class NodeType : u32
{
	Fn,

	Declaration,

	Assignment,

	BinExpr,

	UnaryExpr,

	VarExpr,

	Integer,

	Call,
};

struct NodePtr
{
	NodeType type;
	void* data;
};

struct AstBinaryExpr
{
	TokenType op;
	NodePtr lhs;
	NodePtr rhs;
};

// An expression denoting a location in memory
//	eg. x, arr[1], ob.x.y.z, (*ptr)
struct AstVarExpr
{
	String identifier;
};


struct AstAssign
{
	AstVarExpr lhs;
	NodePtr expr;
};

struct AstDecl
{
	String identifier;
	String type;

	NodePtr valueExpr; // Expression dictating the value of the declaration.
					   // valueExpr.data may be null.
};

struct AstInteger
{
	u64 value;
};

struct AstBlock
{
	Vector<NodePtr> statements;
};

struct AstFn
{
	String name;
	Array<AstDecl> parameters;
	AstBlock block;
};

