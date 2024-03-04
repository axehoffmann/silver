#include "typechecker.hpp"
#include "ast.hpp"
#include "cdebug.hpp"

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
void resolveType(TypeTable* table, TypeRef& ty) 
{
    if (!ty.resolved)
    {
        // #TODO: update fetchInfo func to try fetch by identifier too
        TypeInfo* tinfo = table->resolveType(ty);
        if (tinfo == nullptr)
        {
            std::cout << "Could not resolve type ";
            printType(ty);
            exit(1);
        }
        ty.val = tinfo;
    }
}

TypeRef checkExpr(TypeTable* table, NodePtr& expr);

TypeRef checkBinExpr(TypeTable* table, AstBinaryExpr* expr)
{
    // #TODO: non-equal numeric types etc
    TypeRef lty = checkExpr(table, expr->lhs);
    TypeRef rty = checkExpr(table, expr->rhs);
    switch (expr->op)
    {
        using enum TokenType;
    case Plus:
    case Subtract:
    case Multiply:
    case Divide:
        if (lty.subty != rty.subty || lty.val != rty.val)
        {
            std::cout << "Operator " << getOpchar(expr->op) << " cannot be used on types ";
            printType(lty);
            std::cout << " and ";
            printType(rty);
            exit(1);
        }
        // #TODO: take larger numeric type of the two
        expr->resultTy = lty;
        return lty;

    // Comparison operators always return booleans
    case Eq:
    case Neq:
    case Lt:
    case Leq:
    case Gt:
    case Geq:
        TypeRef boolTy = table->fetchType(Bool, TypeAnnotation::Value);
        expr->resultTy = boolTy;
        return boolTy;

    case And:
    case Or:
        boolTy = table->fetchType(Bool, TypeAnnotation::Value);
        if (lty != boolTy || rty != boolTy)
        {
            std::cout << "Operator '" << getOpchar(expr->op) << "' requires both operands to be of type bool."
                << " Instead, types found were ";
            printType(lty);
            std::cout << " and ";
            printType(rty);
            exit(1);
        }
        return boolTy;

    case Modulo:
        // #TODO: modulo requires int types always
        return table->fetchType(I32, TypeAnnotation::Value);
    default:
        std::cout << "Invalid binary operator\n";
        exit(1);
    }
}

// Type-checks an expression, and returns the resulting type
TypeRef checkExpr(TypeTable* table, NodePtr& expr)
{
    switch (expr.type)
    {
    case NodeType::BinExpr:
        return checkBinExpr(table, expr.binexpr);
    case NodeType::IfExpr:
        TypeRef tty = checkExpr(table, expr.ifexpr->trueVal);
        TypeRef fty = checkExpr(table, expr.ifexpr->falseVal);
        TypeRef condTy = checkExpr(table, expr.ifexpr->condition);

        if (tty != fty)
        {
            std::cout << "If-expressions require the type of "
                << "each result to be equal. Instead found types ";
            printType(tty);
            std::cout << " and ";
            printType(fty);
            exit(1);
        }
        if (condTy != table->fetchType(TokenType::Bool, TypeAnnotation::Value))
        {
            std::cout << "If-expression condition must be type bool."
                << " Instead found ";
            printType(condTy);
            exit(1);
        }
        expr.ifexpr->resultTy = tty;
        return tty;
    case NodeType::Integer:
        return table->fetchType(TokenType::I32, TypeAnnotation::Value);
    }
    
    std::cout << "Cannot typecheck expression node of type " << static_cast<u32>(expr.type);
    exit(1);
}

void checkDecl(TypeTable* table, AstDecl* decl)
{
    if (decl->valueExpr.data == nullptr)
    {
        return;
    }

    TypeRef exprType = checkExpr(table, decl->valueExpr);
    cascadeTypes(decl->type, exprType);
}


void checkBlock(TypeTable* table, TypeRef& returnType, AstBlock& block);

void checkIf(TypeTable* table, TypeRef& returnType, AstIf* ifb)
{
    TypeRef condTy = checkExpr(table, ifb->condition); 
    if (condTy != table->fetchType(TokenType::Bool, TypeAnnotation::Value))
    {
        std::cout << "If-statement condition must be of type bool, instead found ";
        printType(condTy);
        exit(1);
    }

    checkBlock(table, returnType, ifb->block);
}

void checkBlock(TypeTable* table, TypeRef& returnType, AstBlock& block)
{
    for (NodePtr& stmt : block.statements)
    {
        switch (stmt.type)
        {
            using enum NodeType;
        case Declaration:
            checkDecl(table, static_cast<AstDecl*>(stmt.data));
            return;
        case Assignment:
        case If:
            checkIf(table, returnType, stmt.ifs);
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
