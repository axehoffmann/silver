// silver.cpp : Defines the entry point for the application.
//

#include "silver.hpp"
#include "iter.hpp"
#include "lex/Tokenise.hpp"

using namespace std;

const char* in = R"(
main: fn()
{
	var: type = var2;
}
)" + '\0';

int main()
{

	Iter<const char> it(in, 0);
	while (true)
	{
		ParseCtx ctx{};
		Token tok(parseToken(it, ctx));
		printTok(tok);

		if (tok.type == TokenType::Eof)
			break;
	}

	return 0;
}
