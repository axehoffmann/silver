#include "cdebug.hpp"

#include <iostream>

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

    std::cout << fetchInfo(ty)->name;
}

void printVarExpr(const AstVarExpr& expr)
{
    std::cout << "vl<" << expr.identifier << ">";
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
        printVarExpr(*static_cast<const AstVarExpr*>(expr.data));
        return;
    case NodeType::Integer:
        printInteger(*static_cast<const AstInteger*>(expr.data));
        return;
    case NodeType::String:
        printString(*static_cast<const AstString*>(expr.data));
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

void printStatement(const NodePtr& stmt, i32 i)
{
    switch (stmt.type)
    {
    case NodeType::Declaration:
        printDecl(*static_cast<const AstDecl*>(stmt.data));
        return;
    case NodeType::Assignment:
        printAssign(*static_cast<const AstAssign*>(stmt.data));
        return;
    case NodeType::Call:
        printCall(*static_cast<const AstCall*>(stmt.data));
        return;
    }

    std::cout << "don't know how to print this statement";
}

void printBlock(const AstBlock& b, i32 i)
{
    indent(i); std::cout << "{\n";

    for (const NodePtr& node : b.statements)
    {
        indent(i + 1); printStatement(node, i + 1); std::cout << "\n";
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
