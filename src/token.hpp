#pragma once

#include "../lib.hpp"

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
    LBrace = 300,            // {
    RBrace = 301,            // }
    LParen = 302,            // (
    RParen = 303,            // )
    LBrack = 304,            // [
    RBrack = 305,            // ]

    Question = 310,            // ?
    Exclaim = 311,            // !
    Star = 312,                // *
    Plus = 313,                // +
    Minus = 314,            // -
    FSlash = 315,            // /
    BSlash = 316,            // \ 
    Assign = 317,            // =

    Add = 320,                // +=
    Multiply = 321,            // *=
    Subtract = 322,            // -=
    Divide = 323,            // /=

    // Comparison operators
    Eq = 350,                // ==
    Neq = 351,                // !=
    Lt = 352,                // <
    Leq = 353,                // <=
    Gt = 354,                // >
    Geq = 355,                // >=

    // Boolean operators
    And = 400,                // &&
    Or = 401,                // ||

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

    String val; // identifiers, string literals

    union
    {
        u64 uint;    // unsigned int literal
        i64 sint;    // signed int literal
        f64 flt;    // float literal value
    };
};

void printTok(const Token& tok);