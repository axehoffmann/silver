#include "silver.hpp"

#include "src/lexer.hpp"
#include "src/parser.hpp"
#include "src/cdebug.hpp"

#include "src/llvmb/Codegen.hpp"

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
    var: i32 = 5420;
    x: i32 = var;
    
    var = x;
    puts("Hello, digital universe");
}
)" + '\0';

#define PARSE     1
#define CODEGEN     0

int main()
{
    // Timers
    f64 initT = 0.0, lexT = 0.0, parseT = 0.0, llvmT = 0.0;

    // Init
    c::time_point sample = c::now();
    initT = timerToMs(sample, c::now());
    Lexer* lex = new Lexer{ in };
    ParseCtx* pctx = new ParseCtx;
    
    /*
    Vector<Token> tkbuf;
    // Lexing
    sample = c::now();
    while (true)
    {
        Token tk = lex->eat();
        tkbuf.push_back(std::move(tk));

        if (tkbuf.back().type == TokenType::Eof)
            break;
    }
    lexT = timerToMs(sample, c::now());


    for (const Token& tk : tkbuf)
    {
        printTok(tk);
    }
    */
    std::cout << "\nAST:\n";
    
    
    // Parsing
    sample = c::now();
    while (parseAst(*lex, *pctx));
    parseT = timerToMs(sample, c::now());

    for (auto& ex : pctx->externals)
    {
        printFnIface(ex); std::cout << '\n';
    }

    for (auto& fn : pctx->functions)
    {
        printFn((*pctx).functions.front()); std::cout << '\n';
    }


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

    delete pctx;
    return 0;
}
