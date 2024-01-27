#include "typesystem.hpp"

// Builtin types are defined in a static seperate table (? for now ?) 
TypeInfo builtinTys[] = {
    { TypeClass::I8,    1, "i8" },
    { TypeClass::I16,    2, "i16" },
    { TypeClass::I32,    4, "i32" },
    { TypeClass::I64,    8, "i64" },

    { TypeClass::U8,    1, "u8" },
    { TypeClass::U16,    2, "u16" },
    { TypeClass::U32,    4, "u32" },
    { TypeClass::U64,    8, "u64" },

    { TypeClass::F32,    4, "f32" },
    { TypeClass::F64,    8, "f64" },

    { TypeClass::Char,    1, "char" },
    { TypeClass::Byte,    1, "byte" },
    { TypeClass::Bool,    1, "bool" },
    { TypeClass::Void,    1, "void" }, // #TODO: void size meaningful ever?
};

TypeInfo* fetchInfo(const TypeRef& ty)
{
    if (!ty.resolved)
        return nullptr;

    return static_cast<TypeInfo*>(ty.val);
}

TypeRef fetchType(const Token& tok, TypeAnnotation subty)
{
    using enum TokenType;

    // Builtin types
    if (toi(tok.type) >= toi(I8) && toi(tok.type) <= toi(Void))
    {
        return TypeRef{ &builtinTys[toi(tok.type) - toi(I8)], true, subty };
    }

    std::cout << "Cannot handle type at: ";
    printTok(tok);
    exit(-1);
}