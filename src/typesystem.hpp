#pragma once

#include "lib.hpp"
#include "token.hpp"

enum class TypeAnnotation : u8
{
    Value,
    Pointer,
    PointerPointer,
    Array,
    Implicit // Implicit type propagation 
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


struct AstDecl;

struct StructType
{
    Vector<AstDecl*> variables;
};


struct TypeInfo
{
    const char* key;
    TypeClass tyclass;
    u32 size; // sz in bytes of ty
    union
    {
        StructType* structure;
    };
};

class TypeTable
{
public:
    TypeTable();
    ~TypeTable();

    TypeRef fetchType(TokenType tok, TypeAnnotation subty);
    TypeInfo* resolveType(const TypeRef& ty);
    void declareType(const char* name, StructType* type);
private:
    TypeInfo* data;
};

TypeInfo* fetchInfo(const TypeRef& ty);
bool operator==(const TypeRef& a, const TypeRef& b);
