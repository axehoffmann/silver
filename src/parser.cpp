#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "token.hpp"
#include "typesystem.hpp"

constexpr i32 MAX_PARAMS = 10;

using enum TokenType;

void parseComplain(const char* str, const Token& tok)
{
    std::cout << "Parse error on line " << tok.ln << ", " << str << '\n';
    printTok(tok);
    exit(-1);
}

void requireNext(Lexer& lx, ParseCtx& ctx, TokenType type)
{
    if (lx.peek(0).type != type)
    {
        std::cout << "angy, expected " << static_cast<u16>(type)
            << ", found " << static_cast<u16>(lx.peek(0).type) << '\n';
        exit(-1);
    }
    lx.skip(1);
}

bool require(Lexer& lx, ParseCtx& ctx, TokenType type, u32 offset)
{
    if (lx.peek(offset).type != type)
    {
        std::cout << "angy, expected " << static_cast<u16>(type)
            << ", found " << static_cast<u16>(lx.peek(offset).type) << '\n';
        exit(-1);
        return false;
    }

    return true;
}

TypeRef parseType(Lexer& lx, ParseCtx& ctx)
{
    TypeAnnotation tag = TypeAnnotation::Value;
    switch (lx.peek(0).type)
    {
    case Star:
    {
        tag = TypeAnnotation::Pointer;
        lx.skip(1);
        break;
    }
    // Array, pointerpointer, etc

    // Value type
    default: break;
    }

    TypeRef ty = fetchType(lx.peek(0), tag);
    lx.skip(1);
    return ty;
}

AstVarExpr parseVarExpr(Lexer& lx, ParseCtx& ctx)
{
    AstVarExpr node{ lx.peek(0).val };
    lx.skip(1);
    // #TODO: handle fn calls, array indexes, etc 
    return node;
}

NodePtr parseExpr(Lexer& lx, ParseCtx& ctx)
{
    switch (lx.peek(0).type)
    {
    case Identifier:
    {
        AstVarExpr* node = allocate<AstVarExpr>(ctx);
        *node = parseVarExpr(lx, ctx);
        return NodePtr{ NodeType::VarExpr, node };
    }
    case IntLiteral:
    {
        AstInteger* in = allocate<AstInteger>(ctx);
        in->value = lx.eat().uint;
        return NodePtr{ NodeType::Integer, in };
    }
    case StringLiteral:
    {
        AstString* str = allocate<AstString>(ctx);
        str->value = lx.eat().val;
        return NodePtr{ NodeType::String, str };
    }
    }
    std::cout << "wahhhhh\n";
    return NodePtr{};
}

NodePtr parseDecl(Lexer& lx, ParseCtx& ctx)
{
    // 0 is an identifier, 1 is :, 2 should be a type
    const char* name = lx.peek(0).val;
    lx.skip(2);
    TypeRef type = parseType(lx, ctx);
    

    NodePtr expr{};
    if (lx.peek(0).type == Assign)
    {
        lx.skip(1);
        expr = parseExpr(lx, ctx);
    }

    AstDecl* node = allocate<AstDecl>(ctx);
    node->identifier = name;
    node->type = type;
    node->valueExpr = expr;
    return NodePtr{ NodeType::Declaration, node };
}

NodePtr parseAssign(Lexer& lx, ParseCtx& ctx)
{
    AstVarExpr lhs = parseVarExpr(lx, ctx);
    requireNext(lx, ctx, Assign);
    NodePtr rhs = parseExpr(lx, ctx);

    AstAssign* node = allocate<AstAssign>(ctx);
    node->lhs = lhs;
    node->expr = rhs;
    return NodePtr{ NodeType::Assignment, node };
}

NodePtr parseCall(Lexer& lx, ParseCtx& ctx)
{
    // 0: ident,  1: (,  2:...
    const char* fn = lx.peek(0).val;
    lx.skip(2);

    Vector<NodePtr> args;

    while (lx.peek(0).type != RParen)
    {
        args.push_back(parseExpr(lx, ctx));
        if (lx.peek(0).type == Comma)
        {
            lx.skip(1);
            continue;
        }
    }
    lx.skip(1);

    AstCall* call = allocate<AstCall>(ctx);
    call->name = fn;
    call->args = std::move(args);
    return NodePtr{ NodeType::Call, call };
}

NodePtr parseStatement(Lexer& lx, ParseCtx& ctx)
{
    // 0 is an identifier
    NodePtr out;
    switch (lx.peek(1).type)
    {
    case LParen:
        out = parseCall(lx, ctx);
        requireNext(lx, ctx, Semi);
        return out;
    case Separator:
        out = parseDecl(lx, ctx);
        requireNext(lx, ctx, Semi);
        return out;
    case Assign:
        out = parseAssign(lx, ctx);
        requireNext(lx, ctx, Semi);
        return out;
    default:
        complain("Expected a statement (e.g. declaration, function call, etc)", lx.peek(0).ln);
    }
}

AstBlock parseBlock(Lexer& lx, ParseCtx& ctx)
{
    requireNext(lx, ctx, LBrace);

    Vector<NodePtr> statements;

    while (true)
    {
        switch (lx.peek(0).type)
        {
        case Identifier:
            NodePtr node = parseStatement(lx, ctx);
            statements.push_back(node);
            break;
        default:
            printTok(lx.peek(0));
            complain("aahh", 0);
            // For loop etc
        }

        if (lx.peek(0).type == RBrace)
        {
            lx.skip(1);
            break;
        }
    }

    return AstBlock{ std::move(statements) };
}

AstFnInterface parseFnIface(Lexer& lx, ParseCtx& ctx)
{
    AstDecl parameters[MAX_PARAMS];
    u32 paramCount = 0;

    // Already checked that 0 is an identifier, 1 is ':', and 2 is fn/externfn.
    const char* name = lx.peek(0).val;
    lx.skip(3);

    requireNext(lx, ctx, LParen);

    // Parse params
    u32 curTok = 0;
    for (u32 i = 0; i < MAX_PARAMS; i++)
    {
        if (lx.peek(curTok).type == RParen)
        {
            lx.skip(1);
            break;
        }

        require(lx, ctx, Identifier, 0);
        const char* name = lx.peek(0).val;
        lx.skip(1);

        requireNext(lx, ctx, Separator);
        TypeRef ty = parseType(lx, ctx);

        new (parameters + paramCount) AstDecl{ name, ty };
        paramCount++;
        
        if (lx.peek(0).type == Comma)
        {
            lx.skip(1);
            continue;
        }

        requireNext(lx, ctx, RParen);
        break;
    }
    Array<AstDecl> params{ paramCount, parameters };

    // Return type
    TypeRef rettype;
    if (lx.peek(0).type == Ret)
    {
        lx.skip(1);
        rettype = parseType(lx, ctx);
    }
    else
    {
        rettype = fetchType(Token{Void}, TypeAnnotation::Value);
    }

    return AstFnInterface{ std::move(name), std::move(params), std::move(rettype) };
}

void parseFunction(Lexer& lx, ParseCtx& ctx)
{
    AstFnInterface iface = parseFnIface(lx, ctx);
    // Emit declaration

    // Parse body
    AstBlock body = parseBlock(lx, ctx);

    ctx.functions.emplace_back(std::move(iface), std::move(body));
    AstFn* fn = &ctx.functions.back();
    ctx.symbols.declare(fn->iface.name, fn->iface.returnType, NodePtr{ NodeType::Fn, fn });
}

void parseExternFn(Lexer& lx, ParseCtx& ctx)
{
    ctx.externals.push_back(parseFnIface(lx, ctx));
    AstFnInterface* iface = &ctx.externals.back();   
    // Add the declaration to the symbol table
    ctx.symbols.declare(iface->name, iface->returnType, NodePtr{ NodeType::Externfn, iface });
    requireNext(lx, ctx, Semi);
}

void parseDeclaration(Lexer& lx, ParseCtx& ctx)
{
    if (lx.peek(1).type != Separator)
    {
        parseComplain("Invalid top-level statement. \nAll top level statements should be declarations of the form 'name: type ...'", lx.peek(0));
    }

    switch (lx.peek(2).type)
    {
    case Identifier:
        // Should be a variable decl with a user-space type.
        // Will be type checked later.
        return;
    case Fn:
        parseFunction(lx, ctx);
        return;
    case Struct:
        // Struct decl
    case Enum:
        // Enum decl
    case Externfn:
        parseExternFn(lx, ctx);
        return;
    default:
        // Invalid token in declaration, expected 'struct', 'fn', 'enum' or a type identifier.
        return;
    }
}

// Parses the token stream to AST until a reasonable stopping point
// ie. a top-level function or other declaration is complete
bool parseAst(Lexer& lx, ParseCtx& ctx)
{
    switch (lx.peek(0).type)
    {
    case TokenType::Identifier:
        parseDeclaration(lx, ctx);
        return true;
    case Eof:
        return false;
    }
}
