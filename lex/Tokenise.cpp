#include "Tokenise.hpp"

#include "Identifiers.hpp"

#include <string>
using namespace std;

const Token errTok{ TokenType::Eof };

Token makeTkn(Iter<const char>& iter, LexCtx& ctx, u8 w, TokenType type)
{
	Token tkn{ type, ctx.curLn, iter.column, w, 1 };
	iter.advance(w);
	return tkn;
}

Token makeId(Iter<const char>& iter, LexCtx& ctx, u8 w, TokenType type)
{
	Token tkn{ type, ctx.curLn, iter.column, w, 1, String(iter.data(), w) };
	iter.advance(w);
	return tkn;
}

Token parseIdOrKeyword(Iter<const char>& iter, LexCtx& ctx)
{
	i32 width = 0;
	while (isalnum(iter.peek(width))) width++;
	
	switch (shashdyn(iter.data(), width))
	{
	// Decls
	case shash("fn"):		return makeTkn(iter, ctx, 2, TokenType::Fn);
	case shash("externfn"): return makeTkn(iter, ctx, 8, TokenType::Externfn);

	// Integrals
	case shash("i8"):	return makeTkn(iter, ctx, 2, TokenType::I8);
	case shash("i16"):	return makeTkn(iter, ctx, 3, TokenType::I16);
	case shash("i32"):	return makeTkn(iter, ctx, 3, TokenType::I32);
	case shash("i64"):	return makeTkn(iter, ctx, 3, TokenType::I64);

	case shash("u8"):	return makeTkn(iter, ctx, 2, TokenType::U8);
	case shash("u16"):	return makeTkn(iter, ctx, 3, TokenType::U16);
	case shash("u32"):	return makeTkn(iter, ctx, 3, TokenType::U32);
	case shash("u64"):	return makeTkn(iter, ctx, 3, TokenType::U64);

	case shash("bool"):	return makeTkn(iter, ctx, 4, TokenType::Bool);
	case shash("char"): return makeTkn(iter, ctx, 4, TokenType::Char);
	case shash("byte"): return makeTkn(iter, ctx, 4, TokenType::Byte);

	default:
		return makeId(iter, ctx, width, TokenType::Identifier);
	}

}

u64 pow10(u32 i)
{
	u64 x = 1;
	while (i--)
	{
		x *= 10;
	}
	return x;
}

Token parseNumeric(Iter<const char>& iter, LexCtx& ctx)
{
	u8 width = 1;
	while (isdigit(iter.peek(width)))
	{
		width++;
	}

	u64 value = 0;

	for (i32 digit = 1; digit <= width; digit++)
	{
		value += u64(iter.peek(width - digit) - '0') * pow10(digit - 1);
	}
	Token tk{ TokenType::IntLiteral, ctx.curLn, iter.column, width, 1, String(), {.uint = value } };
	iter.advance(width);
	return tk;
}

Token parseToken(Iter<const char>& iter, LexCtx& ctx)
{
	switch (iter.peek(0))
	{
	case '\n':
		ctx.curLn++;
		iter.column = 0;
		[[fallthrough]];
	case ' ':
	case '\t':
		iter.advance(1);
		return parseToken(iter, ctx);

	case ':':
		return makeTkn(iter, ctx, 1, TokenType::Separator);
	case '=':
		return makeTkn(iter, ctx, 1, TokenType::Assign);
	case ',':
		return makeTkn(iter, ctx, 1, TokenType::Comma);
	case ';':
		return makeTkn(iter, ctx, 1, TokenType::Semi);
	case '{':
		return makeTkn(iter, ctx, 1, TokenType::LBrace);
	case '}':
		return makeTkn(iter, ctx, 1, TokenType::RBrace);
	case '(':
		return makeTkn(iter, ctx, 1, TokenType::LParen);
	case ')':
		return makeTkn(iter, ctx, 1, TokenType::RParen);
	case '[':
		return makeTkn(iter, ctx, 1, TokenType::LBrack);
	case ']':
		return makeTkn(iter, ctx, 1, TokenType::RBrack);

	case '*':
		return makeTkn(iter, ctx, 1, TokenType::Star);

	case '-':
		switch (iter.peek(1))
		{
		case '>':
			return makeTkn(iter, ctx, 2, TokenType::Ret);
		}
		return makeTkn(iter, ctx, 1, TokenType::Minus);

	case 0:
		return Token{ TokenType::Eof, iter.position(), 1 };

	default:
		if (isalpha(iter.peek(0)))
		{
			return parseIdOrKeyword(iter, ctx);
		}
		if (isdigit(iter.peek(0)))
		{
			return parseNumeric(iter, ctx);
		}
	};

	std::cout << "Unknown tkn :" << iter.peek(0) << '\n';
	exit(-1);
}