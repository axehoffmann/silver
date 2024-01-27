#pragma once

#include "token.hpp"
#include "silver.hpp"

constexpr u32 LEX_BUFFER_SIZE = 8;

// The lexer maintains a ring buffer of tokens and lazily lexes
// as the parser does its thing. saw this idea on the interwebs and just
// thought why not. (reduces mem usage)
class Lexer
{
public:
    Lexer(const char* source);

    // Eats and returns the current tkn.
    Token eat();
    
    // Peek at an upcoming token. offset MUST be less than LEX_BUFFER_SIZE.
    Token peek(u8 offset);

    // Skips the lexer 'offset' tokens ahead.
    void skip(u8 offset);
private:
    void repopulateBuffer();

    // Lexes a single token and advances the iterator.
    Token lexToken();

    Token makeTkn(u8 width, TokenType type);
    Token makeId(u8 width);

    Token lexIdOrKeyword();
    Token lexNumeric();
    Token lexBasicStringLiteral();

    // ring buffer state
    Token buffer[LEX_BUFFER_SIZE];
    u32 readIdx;
    u32 writeIdx;

    // file id
    u32 fid;

    // read state
    const char* src;
    u64 offset;
    u32 curLn;
    u16 curCol;
};



