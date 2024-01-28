#include "ast.hpp"

#include "stb_ds.h"

void SymbolTable::enter(AstBlock* block)
{
    if (block != nullptr && block->scope != nullptr)
    {
        pos = block->scope;
        return;
    }

    // Add a scope for the block
    Scope* scope = alloc->alloc<Scope>();
    scope->table = nullptr;
    scope->parent = pos;
    if (block != nullptr)
        block->scope = scope;

    pos = scope;
}

void SymbolTable::exit()
{
    // Exits the current scope
    hmfree(pos->table);
    pos = pos->parent;
}

void SymbolTable::declare(const char* key, TypeRef type, NodePtr node)
{
    // Note that we are using a standard hash map as opposed to a string map
    // This is becasue our idents have already been unique'd by the StringHeap.
    // We can use the pointers themselves for the lookup more cheaply.
    Symbol s{ key, type, node };
    hmputs(pos->table, s);
}

Symbol* SymbolTable::lookup(const char* key)
{
    Scope* cur = pos;
    while (cur != nullptr)
    {
        Symbol* symbol = hmgetp_null(cur->table, key);
        if (symbol != nullptr)
            return symbol;
        cur = cur->parent;
    }

    return nullptr;
    // #TODO: external lookup
}
