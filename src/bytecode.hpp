#pragma once

#include "lib.hpp"

enum Instruction : u8
{
    End, // End of block

    Load,
    Store,
    Alloca,
    Deref, // Array/struct indexing
    
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
    Zwide, // Zero-extension to widen int
    Swide, // Sign-extension to widen signed int
    Trunc, // Truncate int
    Ftoi,
    Itof,

    // Special ops
    Atomic, // rmw - eg.  Atomic Add A B
};
