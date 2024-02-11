#include "typechecker.hpp"
#include "ast.hpp"
#include "cdebug"

#include <iostream>

void cascadeTypes(TypeRef& expected, TypeRef& found)
{
    // Cascade expression type to implicit type
    if (expected.subty == TypeAnnotation::Implicit)
    {
        expected.subty = found.subty;
        // Type is guarranteed to be resolved by now 
        expected.resolved = true;
        expected.val = found.val;
        return;
    }

    // Expect type equality by now 
    // #TODO: implicit conversion mechanics could go here? 
    if (expected.val != found.val || expected.subty != found.subty)
    {
        // #TODO: need to have line number etc here for error msgs
        std::cout << "Cannot implicitly convert ";
        printType(found);
        std::cout << " to ";
        printType(expected);
        exit(1);
    }
}

// Resolve a type if it was not resolved during parsing
void resolveType() {}

TypeRef checkExpr(NodePtr& expr)
{

}

void checkDecl(AstDecl* decl)
{
    if (decl->valueExpr.data == nullptr)
    {
        return;
    }

    TypeRef exprType = checkExpr(decl->valueExpr);

}

void checkBlock(TypeRef& returnType, AstBlock& block)
{
    for (NodePtr& stmt : block.statements) 
    {
        switch (stmt.type)
        {
            using enum NodeType;
        case Declaration:
            checkDecl(static_cast<AstDecl*>(stmt.data));
            return;
        case Assignment:
        case If:
        case Call:
        default:
            std::cout << "Invalid AST statement type: " 
                << static_cast<u32>(stmt.type) <<"\n";
            exit(1);
        }
    }
}

void checkFn(AstFn& fn)
{
    
}

void checkTypes(ParseCtx& ctx)
{

}
