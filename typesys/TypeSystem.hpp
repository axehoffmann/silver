#pragma once

#include "lib.hpp"
#include "lex/Token.hpp"

enum class TypeAnnotation : u8
{
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
    Fn,
    Struct,
    Enum,
    Builtin
};

struct TypeInfo
{
    TypeClass tyclass;
};

TypeRef fetchType()
