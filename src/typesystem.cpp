#include "typesystem.hpp"

#include "stb_ds.h"

// Builtin types are defined in a static seperate table (? for now ?) 
TypeInfo builtinTys[] = {
    {  "i8",  TypeClass::I8,     1,},
    { "i16",  TypeClass::I16,    2,},
    { "i32",  TypeClass::I32,    4,},
    { "i64",  TypeClass::I64,    8,},
     
    {  "u8",  TypeClass::U8,     1,},
    { "u16",  TypeClass::U16,    2,},
    { "u32",  TypeClass::U32,    4,},
    { "u64",  TypeClass::U64,    8,},
     
    { "f32",  TypeClass::F32,    4,},
    { "f64",  TypeClass::F64,    8,},
     
    { "char", TypeClass::Char,   1, },
    { "byte", TypeClass::Byte,   1, },
    { "bool", TypeClass::Bool,   1, },
    { "void", TypeClass::Void,   1, }, // #TODO: void size meaningful ever?
};

TypeInfo* fetchInfo(const TypeRef& ty)
{
    if (!ty.resolved)
        return nullptr;

    return static_cast<TypeInfo*>(ty.val);
}

TypeTable::TypeTable() :
    data(nullptr)
{}

TypeTable::~TypeTable()
{
    hmfree(data);
}

TypeRef TypeTable::fetchType(const TokenType& tok, TypeAnnotation subty)
{
    using enum TokenType;

    // Builtin types
    if (toi(tok.type) >= toi(I8) && toi(tok.type) <= toi(Void))
    {
        return TypeRef{ &builtinTys[toi(tok.type) - toi(I8)], true, subty };
    }

    if (tok.type != Identifier)
    {
        std::cout << "Cannot handle type at: ";
        printTok(tok);
        exit(-1);
    }

    // Lookup in user types table
    i32 t = hmgeti(data, tok);
    if (t != -1)
    {
        return TypeRef{ &data[t], true, subty};
    }

    std::cout << "Cannot handle type at: ";
    printTok(tok);
    exit(-1);
}

void TypeTable::declareType(const char* name, StructType* type)
{
    TypeInfo t{ name, TypeClass::Struct, 0, type };
    hmputs(data, t);
}
