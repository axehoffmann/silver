#include "Parse.hpp"
#include "../iter.hpp"
#include "Ast.hpp"
#include "../lex/Token.hpp"

constexpr i32 MAX_PARAMS = 10;

using enum TokenType;

void requireNext(Iter<Token>& iter, ParseCtx& ctx, TokenType type)
{
	if (iter.peek(0).type != type)
	{
		std::cout << "angy, expected " << static_cast<u16>(type)
			<< ", found " << static_cast<u16>(iter.peek(0).type) << '\n';
	}
	iter.advance(1);
}

void require(Iter<Token>& iter, ParseCtx& ctx, TokenType type, u32 offset)
{
	if (iter.peek(offset).type != type)
	{
		std::cout << "angy, expected " << static_cast<u16>(type)
			<< ", found " << static_cast<u16>(iter.peek(offset).type) << '\n';
	}
}

AstVarExpr parseVarExpr(Iter<Token>& iter, ParseCtx& ctx)
{
	AstVarExpr node{ iter.peek(0).val };
	iter.advance(1);
	// TODO: handle fn calls, array indexes, etc 
	return node;
}

NodePtr parseExpr(Iter<Token>& iter, ParseCtx& ctx)
{
	switch (iter.peek(0).type)
	{
	case Identifier:
	{
		AstVarExpr* node = allocate<AstVarExpr>(ctx);
		*node = parseVarExpr(iter, ctx);
		return NodePtr{ NodeType::VarExpr, node };
	}
	case IntLiteral:
	{
		AstInteger* in = allocate<AstInteger>(ctx);
		in->value = iter.peek(0).uint;
		iter.advance(1);
		return NodePtr{ NodeType::Integer, in };
	}
	}
	std::cout << "wahhhhh\n";
	return NodePtr{};
}

NodePtr parseDecl(Iter<Token>& iter, ParseCtx& ctx)
{
	// 0 is an identifier, 1 is :, 2 should be a type
	require(iter, ctx, Identifier, 2);

	String name{ iter.peek(0).val };
	String type{ iter.peek(2).val };

	iter.advance(3);

	NodePtr expr{};
	if (iter.peek(0).type == Assign)
	{
		iter.advance(1);
		expr = parseExpr(iter, ctx);
	}

	AstDecl* node = allocate<AstDecl>(ctx);
	node->identifier = name;
	node->type = type;
	node->valueExpr = expr;
	return NodePtr{ NodeType::Declaration, node };
}

NodePtr parseAssign(Iter<Token>& iter, ParseCtx& ctx)
{
	AstVarExpr lhs = parseVarExpr(iter, ctx);
	requireNext(iter, ctx, Assign);
	NodePtr rhs = parseExpr(iter, ctx);

	AstAssign* node = allocate<AstAssign>(ctx);
	node->lhs = lhs;
	node->expr = rhs;
	return NodePtr{ NodeType::Assignment, node };
}

NodePtr parseCall(Iter<Token>& iter, ParseCtx& ctx)
{
	String fn{ iter.peek(0).val };
	iter.advance(2);

	Vector<NodePtr> args;

	while (iter.peek(0).type != RParen)
	{
		args.push_back(parseExpr(iter, ctx));
		if (iter.peek(0).type == Comma)
		{
			iter.advance(1);
			continue;
		}
	}
	iter.advance(1);

	AstCall* call = allocate<AstCall>(ctx);
	call->name = fn;
	call->args = std::move(args);
	return NodePtr{ NodeType::Call, call };
}

NodePtr parseStatement(Iter<Token>& iter, ParseCtx& ctx)
{
	// 0 is an identifier
	NodePtr out;
	switch (iter.peek(1).type)
	{
	case LParen:
		out = parseCall(iter, ctx);
		requireNext(iter, ctx, Semi);
		return out;
	case Separator:
		out = parseDecl(iter, ctx);
		requireNext(iter, ctx, Semi);
		return out;
	case Assign:
		out = parseAssign(iter, ctx);
		requireNext(iter, ctx, Semi);
		return out;
	default:
		complain("Expected a statement (e.g. declaration, function call, etc)", iter.peek(0).ln);
	}
}

AstBlock parseBlock(Iter<Token>& iter, ParseCtx& ctx)
{
	requireNext(iter, ctx, LBrace);

	Vector<NodePtr> statements;

	while (true)
	{
		switch (iter.peek(0).type)
		{
		case Identifier:
			NodePtr node = parseStatement(iter, ctx);
			statements.push_back(node);
			break;
		default:
			printTok(iter.peek(0));
			complain("aahh", 0);
			// For loop etc
		}

		if (iter.peek(0).type == RBrace)
		{
			iter.advance(1);
			break;
		}
	}

	return AstBlock{ std::move(statements) };
}

AstFnInterface parseFnIface(Iter<Token>& iter, ParseCtx& ctx)
{
	AstDecl parameters[MAX_PARAMS];
	u32 paramCount = 0;

	// Already checked that 0 is an identifier, 1 is ':', and 2 is fn/externfn.
	String name{ iter.peek(0).val };
	iter.advance(3);

	requireNext(iter, ctx, LParen);

	// Parse params
	u32 curTok = 0;
	for (u32 i = 0; i < MAX_PARAMS; i++)
	{
		if (iter.peek(curTok).type == RParen)
		{
			iter.advance(1);
			break;
		}
		require(iter, ctx, Identifier, 0);
		require(iter, ctx, Separator, 1);
		require(iter, ctx, Identifier, 2);

		new (parameters + paramCount) AstDecl{ iter.peek(0).val, iter.peek(1).val };
		paramCount++;
		iter.advance(3);
		if (iter.peek(0).type == Comma)
		{
			iter.advance(1);
			continue;
		}

		requireNext(iter, ctx, RParen);
		break;
	}
	Array<AstDecl> params{ paramCount, parameters };

	// Return type

	return AstFnInterface{ std::move(name), std::move(params), String("void", 4) };
}

void parseFunction(Iter<Token>& iter, ParseCtx& ctx)
{
	AstFnInterface iface = parseFnIface(iter, ctx);
	// Emit declaration

	// Parse body
	AstBlock body = parseBlock(iter, ctx);

	ctx.functions.emplace_back(std::move(iface), std::move(body));
}

void parseExternFn(Iter<Token>& iter, ParseCtx& ctx)
{

}

void parseDeclaration(Iter<Token>& iter, ParseCtx& ctx)
{
	if (iter.peek(1).type != TokenType::Separator)
		// TODO: Complain
		return;


	switch (iter.peek(2).type)
	{
	case TokenType::Identifier:
		// Should be a variable decl with a user-space type.
		// Will be type checked later.
		return;
	case TokenType::Fn:
		parseFunction(iter, ctx);
		return;
	case TokenType::Struct:
		// Struct decl
	case TokenType::Enum:
		// Enum decl
	case TokenType::Externfn:
		parseExternFn(iter, ctx);
		return;
	default:
		// Invalid token in declaration, expected 'struct', 'fn', 'enum' or a type identifier.
		return;
	}
}

// Parses the token stream to AST until a reasonable stopping point
// ie. a top-level function or other declaration is complete
bool parseAst(Iter<Token>& iter, ParseCtx& ctx)
{
	switch (iter.peek(0).type)
	{
	case TokenType::Identifier:
		parseDeclaration(iter, ctx);
		return true;
	case Eof:
		return false;
	}
}