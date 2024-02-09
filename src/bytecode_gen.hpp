#pragma once

#include "allocators.hpp"
#include "lib.hpp"
#include "bytecode.hpp"

enum class Register : u8
{
    R0, R1, R2, R3, R4, R5,
    R6, R7, R8, R9, R10, R11
};

struct BytecodeWriter
{
    BlockArray<u64> fnLocs;
    u8 buffer[1 << 20];
    u64 offset;
    u8 currentRegSize;

    void op(Instruction inst);
    void info(u8 imm1, u8 imm2); // Whether immediate values are used for this op
    void reg(u64 num);
    void immi(u64 val);
    void immf(f64 val);
    void call(void* fndesc, Vector<Register> values);
    void phi(Register out, Vector<i64> predecessors, Vector<Register> values);
};
