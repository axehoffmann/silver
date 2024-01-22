#include "../iter.hpp"
#include "Ast.hpp"
#include "../lex/Token.hpp"

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
		// Function declaration
		return;
	case TokenType::Struct:
		// Struct decl
	case TokenType::Enum:
		// Enum decl
	default:
		// Invalid token in declaration, expected 'struct', 'fn', 'enum' or a type identifier.
	}
}

// Parses the token stream to AST until a reasonable stopping point
// ie. a top-level function or type declaration is complete
void parseAst(Iter<Token>& iter, ParseCtx& ctx)
{
	switch (iter.peek(0).type)
	{
	case TokenType::Identifier:
		return parseDeclaration(iter, ctx);
	}
}