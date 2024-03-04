#pragma once

#include "token.hpp"
#include "lib.hpp"
#include "silver.hpp"
#include "typesystem.hpp"
#include "allocators.hpp"

enum class NodeType : u32
{
    Fn,
    Externfn,
    Struct,

    Declaration,
    Assignment,

    If,

    BinExpr,
    UnaryExpr,
    VarExpr,
    IfExpr,
    Integer,
    String,
    Boolean,

    Call
};

struct AstStruct;
struct AstFn;
struct AstFnInterface;
struct AstBinaryExpr;
struct AstVarExpr;
struct AstAssign;
struct AstDecl;
struct AstInteger;
struct AstString;
struct AstCall;
struct AstIf;
struct AstIfExpr;
struct AstBoolean;

struct NodePtr
{
    NodeType type;
    union
    {
        void* data;
        AstStruct* stct;
        AstFn* fn;
        AstFnInterface* fni;
        AstBinaryExpr* binexpr;
        AstVarExpr* varexpr;
        AstAssign* assign;
        AstDecl* decl;
        AstInteger* integer;
        AstString* string;
        AstBoolean* boolean;
        AstCall* call;
        AstIf* ifs;
        AstIfExpr* ifexpr;
    };
};

struct Symbol
{
    const char* key;
    TypeRef type; // The type of the symbol's value.
    NodePtr node; // AST node that declares this symbol
};
struct Scope
{
    Symbol* table;
    Scope* parent;
};

struct AstBinaryExpr
{
    TokenType op;
    NodePtr lhs;
    NodePtr rhs;
    TypeRef resultTy;
};

// An expression denoting a location in memory
//    eg. x, arr[1], ob.x.y.z, (*ptr)
struct AstVarExpr
{
    const char* identifier;
    TypeRef resultTy;
};


struct AstAssign
{
    AstVarExpr lhs;
    NodePtr expr;
};

struct AstDecl
{
    const char* identifier;
    TypeRef type;

    NodePtr valueExpr; // Expression dictating the value of the declaration.
    // valueExpr.data may be null.
};

struct AstStruct
{
    const char* identifier;
    Vector<AstDecl> members;
};

struct AstInteger
{
    u64 value;
};

struct AstBoolean
{
    bool value;
};

struct AstString
{
    const char* value;
};

struct AstBlock
{
    Vector<NodePtr> statements;
    Scope* scope;
};

struct AstIf
{
    AstBlock block;
    NodePtr condition;
};

struct AstIfExpr
{
    NodePtr condition;
    NodePtr trueVal;
    NodePtr falseVal;
    TypeRef resultTy;
};

struct AstFnInterface
{
    const char* name;
    Array<AstDecl> parameters;
    TypeRef returnType;
};

struct AstFn
{
    AstFnInterface iface;
    AstBlock block;
};

struct AstCall
{
    const char* name;
    Vector<NodePtr> args;
    TypeRef resultTy;
};

AstFnInterface& getInterface(Symbol* symbol);

class SymbolTable
{
public:    
    // Provides stateful methods to walk the table
    void enter(AstBlock* block);
    void exit();
    void declare(const char* key, TypeRef type, NodePtr node);
    Symbol* lookup(const char* key);

    BlockAllocator* alloc;
private:
    Scope* root = nullptr;
    Scope* pos = nullptr;
};

struct ParseCtx
{
    // Declared functions in this file
    BlockArray<AstFn> functions;

    // Functions linked against but not compiled here. e.g C functions
    BlockArray<AstFnInterface> externals;

    BlockArray<AstStruct> structs;

    // Global variables declared in this file
    // Vector<AstDecl> variables;

    SymbolTable symbols;
    TypeTable types;

    char buffer[1 << 20];
    u64 bptr = 0;
};


template <typename T>
T* allocate(ParseCtx& ctx)
{
    T* loc = reinterpret_cast<T*>(&(ctx.buffer[ctx.bptr]));
    ctx.bptr += sizeof(T);
    std::construct_at(loc);
    return loc;
}
