#include "silver.hpp"
#include "iter.hpp"
#include "lex/Tokenise.hpp"

#include "parse/Parse.hpp"
#include "parse/AstPrint.hpp"

#include "Identifiers.hpp"

#include "codegen/genllvm/Codegen.hpp"

#include <chrono> 

using namespace std;
namespace ch = chrono;
using c = chrono::high_resolution_clock;

void complain(const char* message, u32 ln)
{
	std::cout << "Parse error on line (" << ln << "):\n    " << message << '\n';
	exit(1);
}

f64 timerToMs(c::time_point a, c::time_point b)
{
	return ch::duration<f64, std::ratio<1, 1000>>(b - a).count();
}

const char* in = R"(

puts: externfn(val: *char) -> i32;

main: fn()
{
	var: type = 5420;
	x: i32 = var;
	
	var = x;
	printf(1, x);
}
)" + '\0';

#define PARSE	 1
#define CODEGEN	 0

int main()
{
	// Timers
	f64 initT = 0.0, lexT = 0.0, parseT = 0.0, llvmT = 0.0;

	// Init
	c::time_point sample = c::now();
	Iter<const char> it(in);
	LexCtx* lctx = new LexCtx{};
	ParseCtx* pctx = new ParseCtx{};
	Vector<Token> tkbuf;
	initT = timerToMs(sample, c::now());
	
	// Lexing
	sample = c::now();
	while (true)
	{
		Token tk = parseToken(it, *lctx);
		tkbuf.push_back(std::move(tk));

		if (tkbuf.back().type == TokenType::Eof)
			break;
	}
	lexT = timerToMs(sample, c::now());


	for (const Token& tk : tkbuf)
	{
		printTok(tk);
	}
	std::cout << "\nAST:\n";
	
	// Parsing
	sample = c::now();
	Iter<Token> tkit(tkbuf.data());
	while (parseAst(tkit, *pctx));
	parseT = timerToMs(sample, c::now());

	printFn((*pctx).functions.front());

	// LLVM
#if CODEGEN == 1
	sample = c::now();
	std::cout << "\nLLVM:\n";
	createModule(*pctx);
	llvmT = timerToMs(sample, c::now());
#endif
	
	printf(R"(----------
	Init:  %f ms
	Lex:   %f ms
	Parse: %f ms
	LLVM:  %f ms
)", initT, lexT, parseT, llvmT);

	delete lctx;
	delete pctx;
	return 0;
}
