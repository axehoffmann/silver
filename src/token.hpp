#pragma once

#include "../lib.hpp"
#include "allocators.hpp"

enum class TokenType : u16
{
    Eof = 0,

    // Constructs
    Struct = 100,            // struct
    Enum = 101,                // enum
    Fn = 102,                // fn
    Externfn = 103,            // externfn

    Identifier = 110,

    Separator = 120,        // : 
    Semi = 121,                // ;
    Comma = 122,            // ,
    Comment = 123,            // //
    Discard = 124,            // _
    Ret = 125,                // ->

    // Flow
    If = 170,                // if
    Else = 171,                // else
    Switch = 172,            // switch
    Return = 173,            // return


    // Builtin types   (note when adding - must be between i8 and void for fast type lookup)
    I8 = 200,                // i8
    I16 = 201,                // i16
    I32 = 202,                // i32
    I64 = 203,                // i64

    U8 = 204,                // u8
    U16 = 205,                // u16
    U32 = 206,                // u32
    U64 = 207,                // u64

    F32 = 208,                // f32
    F64 = 209,                // f64

    Char = 210,                // char
    Byte = 211,                // byte
    Bool = 212,                // bool
    Void = 213,                // void


    // Symbols
    LBrace = 250,            // {
    RBrace = 251,            // }
    LParen = 252,            // (
    RParen = 253,            // )
    LBrack = 254,            // [
    RBrack = 255,            // ]

    Assign = 260,            // =
    Question = 261,          // ?
    Exclaim = 262,           // !

    // Binary operators
    Plus = 310,              // +
    Minus = 311,             // -
    Star = 312,              // *
    FSlash = 313,            // /
    Modulo = 314,            // %

    // Comparison operators
    Eq = 340,                // ==
    Neq = 341,               // !=
    Lt = 342,                // <
    Leq = 343,               // <=
    Gt = 344,                // >
    Geq = 345,               // >=
    // Boolean operators
    And = 360,               // &&
    Or = 361,                // ||

    // Unary operators
    Addressof = 380,         // ^

    // Special expressions
    Ifx = 390,               // ifx
    Then = 391,              // then
    Matchx = 392,            // matchx

    // Assignment operators
    Add = 500,                // +=
    Multiply = 501,            // *=
    Subtract = 502,            // -=
    Divide = 503,            // /=

    // Literals
    IntLiteral = 700,        // 0-9+

    FloatLiteral = 710,        // 0-9+[.0-9+]f

    StringLiteral = 720,    // "abcxyz123"

    TrueLiteral = 730,        // true
    FalseLiteral = 731,        // false

};

u16 toi(TokenType ty);

#include <iostream>
#include <format>

struct Token
{
    TokenType type;
    u32 ln; // dist into file
    u16 horizontal;
    u8 count; // width of token

    u32 src; // src file id

    union
    {
        const char* val; // identifiers, string literals
        u64 uint;    // unsigned int literal
        i64 sint;    // signed int literal
        f64 flt;    // float literal value
    };
};

void printTok(const Token& tok);