#include "Tokenise.hpp"

#include <string>
using namespace std;

constexpr i32 MAX_KW_WIDTH = 6;

const Token errTok{ TokenType::Eof };

Token makeTkn(Iter<const char>& iter, u32 w, TokenType type)
{
	Token tkn{ type, iter.position(), w, 1 };
	iter.advance(w);
	return tkn;
}

Token makeId(Iter<const char>& iter, u32 w, TokenType type)
{
	Token tkn{ type, iter.position(), w, 1, String(iter.data(), w) };
	iter.advance(w);
	return tkn;
}

Token parseIdOrKeyword(Iter<const char>& iter, ParseCtx& ctx)
{
	i32 width = 0;
	while (isalnum(iter.peek(width))) width++;
	
	if (width)
	// We know its not a kw if above a certain width
	if (width > MAX_KW_WIDTH)
	{
		return makeId(iter, width, TokenType::Identifier);
	}

	if (std::strncmp(iter.data(), "fn", 2) == 0)
	{
		return makeTkn(iter, 2, TokenType::Fn);
	}

	return makeId(iter, width, TokenType::Identifier);
}

Token parseToken(Iter<const char>& iter, ParseCtx& ctx)
{
	switch (iter.peek(0))
	{
	case ' ':
	case '\t':
	case '\n':
		iter.advance(1);
		return parseToken(iter, ctx);
	case ':':
		return makeTkn(iter, 1, TokenType::Separator);
	case '=':
		return makeTkn(iter, 1, TokenType::Assign);
	case ';':
		return makeTkn(iter, 1, TokenType::Semi);
	case '{':
		return makeTkn(iter, 1, TokenType::LBrace);
	case '}':
		return makeTkn(iter, 1, TokenType::RBrace);
	case '(':
		return makeTkn(iter, 1, TokenType::LParen);
	case ')':
		return makeTkn(iter, 1, TokenType::RParen);
	case '[':
		return makeTkn(iter, 1, TokenType::LBrack);
	case ']':
		return makeTkn(iter, 1, TokenType::RBrack);

	case 0:
		return Token{ TokenType::Eof, iter.position(), 1 };
	default:
		if (isalpha(iter.peek(0)))
		{
			return parseIdOrKeyword(iter, ctx);
		}
	};
}