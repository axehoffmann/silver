#include "lexer.hpp"

#include "utils.hpp"

#include <string>
using namespace std;

const Token errTok{ TokenType::Eof };

using enum TokenType;

Lexer::Lexer(const char* source, StringHeap* strHeap) :
    readIdx(0),
    writeIdx(0),
    fid(1),
    src(source),
    offset(0),
    curLn(0),
    curCol(0),
    strings(strHeap)
{
    // Populate buffer initially
    for (i32 i = 0; i < LEX_BUFFER_SIZE; i++)
    {
        buffer[i] = lexToken();
    }
}

// Fill up the next x tokens in the buffer if required.
void Lexer::repopulateBuffer()
{
    while (writeIdx != readIdx)
    {
        buffer[writeIdx] = lexToken();
        writeIdx = (writeIdx + 1) % LEX_BUFFER_SIZE;
    }
}

Token Lexer::eat()
{
    repopulateBuffer();

    Token tkn = buffer[readIdx];
    readIdx = (readIdx + 1) % LEX_BUFFER_SIZE;
    return tkn;
}

Token Lexer::peek(u8 offset)
{
    repopulateBuffer();
    return buffer[(readIdx + offset) % LEX_BUFFER_SIZE];
}

void Lexer::skip(u8 offset)
{
    readIdx = (readIdx + offset) % LEX_BUFFER_SIZE;
}

Token Lexer::makeTkn(u8 w, TokenType type)
{
    Token tkn{ type, curLn, curCol, w, fid };
    offset += w;
    curCol += w;
    return tkn;
}

Token Lexer::makeId(u8 w)
{
    Token tkn{ Identifier, curLn, curCol, w, fid, { .val=strings->get(src + offset, w) } };
    offset += w;
    curCol += w;
    return tkn;
}

Token Lexer::lexBasicStringLiteral()
{
    u16 width = 1;
    while (true)
    {
        // #SPEED: SIMD scan?
        char ch = *(src + offset + width);
        if (ch == '"')
            break;

        if (ch == '\n')
        {
            // #TODO: proper error handling
            std::cout << "Invalid string literal on line " << curLn << '\n'; 
            exit(-1);
        }

        width++;
    }

    Token tkn{ StringLiteral, curLn, curCol, 1, fid, strings->get(src + offset + 1, width - 1) };
    offset += width + 1;
    curCol += width + 1;
    return tkn;
}

Token Lexer::lexIdOrKeyword()
{
    i32 width = 1;
    // Find end of id
    while (isalnum(*(src + offset + width))) width++;

    switch (shashdyn(src + offset, width))
    {
        // Decls
    case shash("fn"):        return makeTkn(2, TokenType::Fn);
    case shash("externfn"): return makeTkn(8, TokenType::Externfn);

        // Integrals
    case shash("i8"):    return makeTkn(2, TokenType::I8);
    case shash("i16"):    return makeTkn(3, TokenType::I16);
    case shash("i32"):    return makeTkn(3, TokenType::I32);
    case shash("i64"):    return makeTkn(3, TokenType::I64);

    case shash("u8"):    return makeTkn(2, TokenType::U8);
    case shash("u16"):    return makeTkn(3, TokenType::U16);
    case shash("u32"):    return makeTkn(3, TokenType::U32);
    case shash("u64"):    return makeTkn(3, TokenType::U64);

    case shash("bool"):    return makeTkn(4, TokenType::Bool);
    case shash("char"): return makeTkn(4, TokenType::Char);
    case shash("byte"): return makeTkn(4, TokenType::Byte);

    default:
        return makeId(width);
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

Token Lexer::lexNumeric()
{
    u8 width = 1;
    while (isdigit(*(src + offset + width))) width++;

    u64 value = 0;

    for (i32 digit = 1; digit <= width; digit++)
    {
        value += (u64(*(src + offset + width - digit)) - '0') * pow10(digit - 1);
    }
    Token tk{ TokenType::IntLiteral, curLn, curCol, width, 1, { .uint = value } };
    offset += width;
    curCol += width;
    return tk;
}

Token Lexer::lexToken()
{
    switch (*(src + offset))
    {
    case '\n':
        curLn++;
        curCol = 0;
        [[fallthrough]];
    case ' ':
    case '    ':
        offset += 1;
        curCol += 1;
        return lexToken();

    case ':':
        return makeTkn(1, Separator);
    case '=':
        return makeTkn(1, Assign);
    case ',':
        return makeTkn(1, Comma);
    case ';':
        return makeTkn(1, Semi);
    case '{':
        return makeTkn(1, LBrace);
    case '}':
        return makeTkn(1, RBrace);
    case '(':
        return makeTkn(1, LParen);
    case ')':
        return makeTkn(1, RParen);
    case '[':
        return makeTkn(1, LBrack);
    case ']':
        return makeTkn(1, RBrack);

    case '+':
        return makeTkn(1, Plus);
    case '*':
        return makeTkn(1, Star);

    case '-':
        switch (*(src + offset + 1))
        {
        case '>':
            return makeTkn(2, TokenType::Ret);
        }
        return makeTkn(1, TokenType::Minus);

    case EOF:
    case 0:
        return Token{ TokenType::Eof };
    case '"':
        return lexBasicStringLiteral();
    default:
        if (isalpha(*(src + offset)))
        {
            return lexIdOrKeyword();
        }
        if (isdigit(*(src + offset)))
        {
            return lexNumeric();
        }
    };

    std::cout << "Unknown tkn :" << *(src + offset) << '\n';
    exit(-1);
}
