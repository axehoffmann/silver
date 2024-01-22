#pragma once

#include "../lib.hpp"

enum class NodeType : u32
{
	Fn,

	Declaration,

	Assignment,

	Expression,

	Call,
};

struct AstNode;

struct AstDecl
{
	String identifier;
	String type;
};

struct AstBlock
{
	Array<NodePtr> statements;
};

struct AstFn
{
	String name;
	Array<AstDecl> parameters;
	AstBlock block;
};

class NodePtr
{
public:
	NodeType type;
private:
	void* data;
};