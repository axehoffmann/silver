// silver.cpp : Defines the entry point for the application.
//

#include "silver.hpp"
#include "iter.hpp"
#include "lex/Tokenise.hpp"

#include "parse/Parse.hpp"
#include "parse/AstPrint.hpp"

using namespace std;

const char* in = R"(
main: fn()
{
	var: type = 5420;
	x: i32 = var;

	x = var2;
}
)" + '\0';

int main()
{

	Iter<const char> it(in);
	ParseCtx* ctx = new ParseCtx;

	Vector<Token> tkbuf;

	while (true)
	{
		tkbuf.push_back(parseToken(it, *ctx));

		if (tkbuf.back().type == TokenType::Eof)
			break;
	}

	for (const Token& tk : tkbuf)
	{
		printTok(tk);
	}
	std::cout << "aaa\n";

	Iter<Token> tkit(tkbuf.data());
	while (parseAst(tkit, *ctx))
	{
	}

	printFn((*ctx).functions.front());

	delete ctx;
	return 0;
}
