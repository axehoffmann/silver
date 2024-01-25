// silver.cpp : Defines the entry point for the application.
//

#include "silver.hpp"
#include "iter.hpp"
#include "lex/Tokenise.hpp"

#include "parse/Parse.hpp"
#include "parse/AstPrint.hpp"

#include "codegen/genllvm/Codegen.hpp"

#include <chrono>

using namespace std;
namespace ch = chrono;
using c = chrono::high_resolution_clock;

const char* in = R"(
main: fn()
{
	var: type = 5420;
	x: i32 = var;
	
	var = x;
}
)" + '\0';

f64 timerToMs(c::time_point a, c::time_point b)
{
	return ch::duration<f64, std::ratio<1, 1000>>(b - a).count();
}

int main()
{
	f64 initT = 0.0;
	f64 lexT = 0.0;
	f64 parseT = 0.0;
	f64 llvmT = 0.0;

	c::time_point sample = c::now();
	Iter<const char> it(in);
	ParseCtx* ctx = new ParseCtx;
	Vector<Token> tkbuf;
	initT = timerToMs(sample, c::now());


	sample = c::now();
	while (true)
	{
		tkbuf.push_back(parseToken(it, *ctx));

		if (tkbuf.back().type == TokenType::Eof)
			break;
	}
	lexT = timerToMs(sample, c::now());


	for (const Token& tk : tkbuf)
	{
		printTok(tk);
	}
	std::cout << "\nAST:\n";


	sample = c::now();
	Iter<Token> tkit(tkbuf.data());
	while (parseAst(tkit, *ctx));
	parseT = timerToMs(sample, c::now());


	printFn((*ctx).functions.front());

	std::cout << "\nLLVM:\n";
	sample = c::now();
	createModule(*ctx);
	llvmT = timerToMs(sample, c::now());

	printf(R"(----------
	Init:  %f ms
	Lex:   %f ms
	Parse: %f ms
	LLVM:  %f ms
)", initT, lexT, parseT, llvmT);

	delete ctx;
	return 0;
}
