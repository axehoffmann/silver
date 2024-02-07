#include "cdebug.hpp"

#include <iostream>

char getOpchar(TokenType tok)
{
    switch (tok)
    {
    case TokenType::Plus:
        return '+';
    case TokenType::Minus:
        return '-';
    case TokenType::Star:
        return '*';
    }

    return '~';
}

void indent(i32 tabs)
{
    for (i32 i = 0; i < tabs; i++)
    {
        std::cout << "    ";
    }
}

void printType(const TypeRef& ty)
{
    if (!ty.resolved)
    {
        std::cout << "Cant debug print an unresolved type";
        return;
    }

    if (ty.subty == TypeAnnotation::Pointer)
        std::cout << '*';

    std::cout << fetchInfo(ty)->key;
}

void printVarExpr(const AstVarExpr& expr)
{
    std::cout << "val<" << expr.identifier << ">";
}

void printInteger(const AstInteger& i)
{
    std::cout << i.value;
}

void printString(const AstString& str)
{
    std::cout << '"' << str.value << '"';
}

void printExpr(const NodePtr& expr)
{
    switch (expr.type)
    {
    case NodeType::VarExpr:
        printVarExpr(*expr.varexpr);
        return;
    case NodeType::Integer:
        printInteger(*expr.integer);
        return;
    case NodeType::String:
        printString(*expr.string);
        return;
    case NodeType::BinExpr:
        const AstBinaryExpr* binop = expr.binexpr;
        std::cout << "(";
        printExpr(binop->lhs);
        std::cout << ' ' << getOpchar(binop->op) << ' ';
        printExpr(binop->rhs);
        std::cout << ")";
        return;
    }

    std::cout << "Don't know how to print this";
}

void printDecl(const AstDecl& decl)
{
    std::cout << "<decl: " << decl.identifier << ": ";
    printType(decl.type);
    if (decl.valueExpr.data != nullptr)
    {
        std::cout << " = ";
        printExpr(decl.valueExpr);
    }
    std::cout << ">";
}

void printAssign(const AstAssign& asgn)
{
    std::cout << "<assign: ";
    printVarExpr(asgn.lhs);
    std::cout << " = ";
    printExpr(asgn.expr);
    std::cout << ">";
}

void printCall(const AstCall& call)
{
    std::cout << "<call: " << call.name << " (";
    for (u32 i = 0; i < call.args.size(); i++)
    {
        printExpr(call.args[i]);
        if (i < call.args.size() - 1)
            std::cout << ", ";
    }
    std::cout << ") >";
}

void printBlock(const AstBlock& b, i32 i);
void printIf(const AstIf& stmt, i32 i)
{
    std::cout << "<if: ";
    printExpr(stmt.condition);
    std::cout << " >:\n";
    printBlock(stmt.block, i + 1);
}

void printStatement(const NodePtr& stmt, i32 i)
{
    switch (stmt.type)
    {
    case NodeType::Declaration:
        printDecl(*stmt.decl);
        return;
    case NodeType::Assignment:
        printAssign(*stmt.assign);
        return;
    case NodeType::Call:
        printCall(*stmt.call);
        return;
    case NodeType::If:
        printIf(*stmt.ifs, i);
        return;
    }

    std::cout << "don't know how to print this statement";
}

void printBlock(const AstBlock& b, i32 i)
{
    indent(i); std::cout << "{\n";

    for (const NodePtr& node : b.statements)
    {
        indent(i + 1); printStatement(node, i); std::cout << "\n";
    }

    indent(i); std::cout << "}\n";
}

void printFnIface(const AstFnInterface& fn)
{
    std::cout << "<func: " << fn.name << "(";
    for (u32 i = 0; i < fn.parameters.size(); i++)
    {
        std::cout << fn.parameters[i].identifier << ": ";
        printType(fn.parameters[i].type);
        if (i < fn.parameters.size() - 1)
            std::cout << ",";
    }
    std::cout << ")> -> ";
    printType(fn.returnType);
    std::cout << '\n';
}

void printFn(const AstFn& fn)
{
    printFnIface(fn.iface);
    printBlock(fn.block, 0);
}
