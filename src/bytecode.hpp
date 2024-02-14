#pragma once

#include "lib.hpp"

// arginfo structure
// 0000   00  0      0 
// ^type      ^imm1  ^imm2 

enum BcType : u8
{
    // Types
    F32, F64,
    I8, I16, I32, I64,
    U8, U16, U32, U64,
};

enum Instruction : u8
{
    End, // End of block

    Load,
    Store,
    Alloca,
    Pmth, // Array/struct indexing

    // Modifiers
    Sse,
    Atmc, // e.g. atmc add i32 *x 

    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,

    // Bit ops
    Lshf,
    Rshf,
    And,
    Or,
    Xor,
    Not,

    // Comparison 
    Eq,
    Neq,
    Lt,
    Lte,
    Gt,
    Gte,

    // Misc ops
    Select,
    Min,
    Max,

    // Control flow
    Jmp,
    Cjmp,
    Switch,
    Ret,
    Call,
    Phi,

    // Conversion ops
    Abs, // sint to uint
    Extz, // Zero-extension to widen int
    Exts, // Sign-extension to widen signed int
    Trunc, // Truncate int
    Ftosi,
    Sitof,
    Ftoui,
    Uitof,
};
