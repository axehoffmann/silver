#include "Token.hpp"

i32 cast(TokenType tk)
{
	return static_cast<i32>(tk);
}

void printTok(const Token& tok)
{
	if (tok.type == TokenType::Identifier)
	{
		std::cout
			<< std::vformat("({}, {}, {}, \"",
				std::make_format_args(cast(tok.type), tok.loc, tok.count))
			<< tok.val.data()
			<< "\")"
			<< '\n';
		return;
	}

	if (tok.type == TokenType::IntLiteral)
	{
		std::cout
			<< std::vformat("({}, {}, {}, \"",
				std::make_format_args(cast(tok.type), tok.loc, tok.count))
			<< tok.uint
			<< "\")"
			<< '\n';
	}

	std::cout
		<< std::vformat("({}, {}, {})",
			std::make_format_args(cast(tok.type), tok.loc, tok.count))
		<< ")"
		<< '\n';
}