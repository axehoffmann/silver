#include "bytecode_gen.hpp"

using enum Instruction;

void BytecodeWriter::op(Instruction inst)
{
    buffer[offset] = inst;
    offset++;
}

void BytecodeWriter::info(u8 imm1, u8 imm2)
{
    u8 inf = 0;
    inf |= imm1;
    inf |= imm2 << 1;
    buffer[offset] = inf;
    offset++;
}

void BytecodeWriter::reg(u64 num)
{
    if (num > (1 << (8 * num)))
        currentRegSize++;

    for (u8 bytenum = 0; bytenum < currentRegSize; bytenum++)
    {
        u8 curByte = (num & (256 << (8 * bytenum)) >> (8 * bytenum)); 
        buffer[offset] = curByte;
        offset++;
    }
}

void BytecodeWriter::immi(u64 val)
{
    
}


