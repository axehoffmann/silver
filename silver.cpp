#include "silver.hpp"

#include "src/lexer.hpp"
#include "src/parser.hpp"
#include "src/cdebug.hpp"

#include "src/llvmb/codegen.hpp"

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

Foo: struct {
    fooby: i32;
    epic: i32;
}

main: fn() {
    var: i32 = 3 - 3 + 6 * 4;
    x: i32 = var;
    
    var = x;
    puts("Hello, digital universe");
    printerton(false, 1);
    printerton(true, 0);
    printerton(false, 1);
}

printerton: fn(cond: bool, msgnum: i32) {
    if cond == true || false {
        var: *char = "Out of order declaration!";
        varB: *char = "Crazy stuff";
        puts(ifx msgnum == 0 then var else varB);
    }
}

)";

#define PARSE       1
#define CODEGEN     1

int main()
{
    // Timers
    f64 initT = 0.0, lexT = 0.0, parseT = 0.0, llvmT = 0.0;

    // ----- Init -----
    c::time_point sample = c::now();
    // Prepare allocators
    BlockAllocator allocator{};
    allocator.initialise();
    StringHeap strings{};
    strings.initialise();

    Lexer* lex = new Lexer{ in, &strings };
    ParseCtx* pctx = new ParseCtx;
    // Create root symbol table for the file
    pctx->symbols.alloc = &allocator;
    pctx->symbols.enter(nullptr); 

    initT = timerToMs(sample, c::now());

    std::cout << "\nAST:\n";
    
    // ----- Parsing -----
    sample = c::now();
    while (parseAst(*lex, *pctx));
    parseT = timerToMs(sample, c::now());

    /*
    for (auto& ex : pctx->externals)
    {
        printFnIface(ex); std::cout << '\n';
    }

    for (auto& fn : pctx->functions)
    { 
        printFn(fn); std::cout << '\n';
    }
    */

    // ----- LLVM -----
#if CODEGEN == 1
    sample = c::now();
    std::cout << "\nLLVM:\n";
    createModule(*pctx);
    llvmT = timerToMs(sample, c::now());
#endif
    
    printf(R"(----------
    Init:  %f ms
    Parse: %f ms
    LLVM:  %f ms
)", initT, parseT, llvmT);

    allocator.dump();
    strings.dump();
    delete pctx;
    return 0;
}
