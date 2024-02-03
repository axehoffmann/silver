#pragma once

#include "lib.hpp"
#include "src/token.hpp"
#include "src/ast.hpp"

enum class TypeAnnotation : u8
{
    Value,
    Pointer,
    PointerPointer,
    Array
};

// Refers to an identifier string, until the typecheck phase, after which it points to an entry in the type table
struct TypeRef
{
    void* val;
    bool resolved;
    TypeAnnotation subty;
};

// Classification of type
enum class TypeClass : u32
{
    // Builtin types
    I8,     // i8
    I16,    // i16
    I32,    // i32
    I64,    // i64

    U8,      // u8
    U16,     // u16
    U32,     // u32
    U64,     // u64

    F32,     // f32
    F64,     // f64

    Char,     // char
    Byte,    // byte
    Bool,    // bool
    Void,     // void

    // User types
    Fn = 100,
    Struct = 101,
    Enum = 102,
};


struct StructType
{
    Vector<AstDecl*> variables;
};


struct TypeInfo
{
    const char* key;
    TypeClass tyclass;
    union
    {
        StructType* structure;
    }
    u32 size; // sz in bytes of ty
};

// #TODO: Bad name, come up with something better
struct SemanticCtx
{
    // The 'local' type table - of the module and all dependencies
    TypeInfo* typeTable;
    SyncedBlockArray<StructType>* globalTypeTable;
};

TypeInfo* fetchInfo(const TypeRef& ty);
TypeRef fetchType(const Token& tok, TypeAnnotation subty);
