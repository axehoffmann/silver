#include "codegen.hpp"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Verifier.h"

// #TODO: investigate llvm/Support/VirtualFileSystem.h


using namespace llvm;

LocalScope::LocalScope()
{
    sh_new_arena(hashTable);
}

void LocalScope::put(const char* name, VarType ty, llvm::Value* inst)
{
    Entry val{ name, ty, inst };
    shputs(hashTable, val);
}

LocalScope::Entry* LocalScope::get(const char* name)
{
    i64 idx = shgeti(hashTable, name);
    if (idx == -1)
    {
        std::cout << "scope lookup fail wahh\n";
    }

    return &hashTable[idx];
}

LocalScope::~LocalScope()
{
    shfree(hashTable);
}

Type* resolveType(LLVMContext& ctx, TypeRef& ty)
{
    // We assume the typeref has been resolved, otherwise we should have had a
    // typecheck error earlier.
    TypeInfo* info = fetchInfo(ty);
    
    switch (info->tyclass)
    {
    case TypeClass::Char:
    case TypeClass::I8:
        if (ty.subty == TypeAnnotation::Pointer)
            return Type::getInt8PtrTy(ctx);
        else
            return Type::getInt8Ty(ctx);
    case TypeClass::I32:
        return Type::getInt32Ty(ctx);
    case TypeClass::Void:
        return Type::getVoidTy(ctx);
    }
    
    std::cout << "cant handle type '" << info->name << "'\n";
    exit(-1);
}

Value* genInteger(LLVMContext& ctx, AstInteger* node)
{
    return ConstantInt::get(Type::getInt32Ty(ctx), node->value);
}


Value* genVarExpr(LLVMContext& ctx, AstVarExpr& node, LocalScope& scope, llvmBuilder& bldr, bool asPtr = false)
{
    LocalScope::Entry* e = scope.get(node.identifier);
    switch (e->type)
    {
    case VarType::Param:
        return e->val;
    case VarType::LocalVar:
        AllocaInst* a = static_cast<AllocaInst*>(e->val);
        // #TODO: i think this is ugly. Try unify concept of loads/stores with various var types.
        if (asPtr)
            return a;
        return bldr.CreateLoad(a->getAllocatedType(), a);
    }
    std::cout << "not good :(\n";
    exit(-1);
}

Value* genStringLiteral(LLVMContext& ctx, AstString* node, LocalScope& scope, llvmBuilder& bldr)
{
    return bldr.CreateGlobalStringPtr(node->value);
}

Function* genFnInterface(LLVMContext& ctx, AstFnInterface& node, Module& modl)
{
    if (strcmp(node.name, "main") == 0)
    {
        // Main function special case. Needs argc, argv, but isn't explicitly declared in language.
        Type* args[] = { Type::getInt32PtrTy(ctx), PointerType::get(Type::getInt8PtrTy(ctx), 0)};
        FunctionType* fntype = FunctionType::get(Type::getInt32Ty(ctx), args, false);
        return Function::Create(fntype, Function::ExternalLinkage, node.name, modl);
    }
    
    Array<Type*> args{ node.parameters.size() };
    for (u32 i = 0; i < node.parameters.size(); i++) 
    {
        args[i] = resolveType(ctx, node.parameters[i].type);
    }
    FunctionType* fntype = FunctionType::get(resolveType(ctx, node.returnType), ArrayRef<Type*>{ args.begin(), args.end() }, false);
    Function* fn = Function::Create(fntype, Function::ExternalLinkage, Twine(node.name), modl);
    return fn;
}

Value* genExpr(LLVMContext& ctx, SymbolTable& sym, NodePtr& node, LocalScope& scope, llvmBuilder& bldr);

Value* genBinOp(LLVMContext& ctx, SymbolTable& sym, AstBinaryExpr* node, LocalScope& scope, llvmBuilder& bldr)
{
    Value* lhs = genExpr(ctx, sym, node->lhs, scope, bldr);
    Value* rhs = genExpr(ctx, sym, node->rhs, scope, bldr);

    switch (node->op)
    {
    case TokenType::Plus:
        return bldr.CreateAdd(lhs, rhs);
    case TokenType::Star:
        return bldr.CreateMul(lhs, rhs);
    case TokenType::Minus:
        return bldr.CreateSub(lhs, rhs);
    case TokenType::FSlash:
        return bldr.CreateFDiv(lhs, rhs);
    case TokenType::Eq:
        return bldr.CreateICmpEQ(lhs, rhs);
    }
    std::cout << "Could not gen LLVM code for binary operator " << toi(node->op) << '\n';
    exit(-1);
}


Value* genExpr(LLVMContext& ctx, SymbolTable& sym, NodePtr& node, LocalScope& scope, llvmBuilder& bldr)
{
    switch (node.type)
    {
    case NodeType::Integer:
        return genInteger(ctx, static_cast<AstInteger*>(node.data));
    case NodeType::VarExpr:
        return genVarExpr(ctx, *static_cast<AstVarExpr*>(node.data), scope, bldr);
    case NodeType::String:
        return genStringLiteral(ctx, static_cast<AstString*>(node.data), scope, bldr);
    case NodeType::BinExpr:
        return genBinOp(ctx, sym, static_cast<AstBinaryExpr*>(node.data), scope, bldr);
    }
    std::cout << "Could not gen LLVM code for expression type " << static_cast<u32>(node.type) << '\n';
    exit(-1);
}

// Generates an alloca at function scope, and a possible store at local scope
AllocaInst* genLocalVariableDecl(LLVMContext& ctx, SymbolTable& sym, LocalScope& scope, AstDecl& decl, llvmBuilder& bldr)
{
    Function* fn = bldr.GetInsertBlock()->getParent();
    IRBuilder<> entry{ &fn->getEntryBlock(), fn->getEntryBlock().begin() };
    AllocaInst* inst = entry.CreateAlloca(resolveType(ctx, decl.type), nullptr, decl.identifier);
    scope.put(decl.identifier, VarType::LocalVar, inst);

    if (decl.valueExpr.data != nullptr)
    {
        bldr.CreateStore(genExpr(ctx, sym, decl.valueExpr, scope, bldr), inst);
    }
    return inst;
}

void genAssign(LLVMContext& ctx, SymbolTable& sym, LocalScope& scope, llvmBuilder& bldr, AstAssign& asgn)
{
    Value* val = genExpr(ctx, sym, asgn.expr, scope, bldr);
    Value* lhs = genVarExpr(ctx, asgn.lhs, scope, bldr, true);
    bldr.CreateStore(val, lhs);
}

Value* genCall(LLVMContext& ctx, SymbolTable& sym, LocalScope& scope, llvmBuilder& bldr, Module& modl, AstCall& call)
{
    // #BAD: alloc
    // #TODO: types resolved in typecheck stage instead of symbol table lookup now ?
    Array<Value*> args{ u32(call.args.size()) };
    Array<Type*> argTys{ u32(call.args.size()) };
    AstFnInterface& fn = getInterface(sym.lookup(call.name));
    for (u32 i = 0; i < call.args.size(); i++)
    {
        argTys[i] = resolveType(ctx, fn.parameters[i].type);
        args[i] = genExpr(ctx, sym, call.args[i], scope, bldr);
    }

    FunctionType* callTy = FunctionType::get(resolveType(ctx, fn.returnType), ArrayRef<Type*>{ argTys.begin(), argTys.end()}, false);
    FunctionCallee target = modl.getOrInsertFunction(call.name, callTy);
    return bldr.CreateCall(target, ArrayRef<Value*>{ args.begin(), args.end() });
}

void genBlock(LLVMContext& ctx, SymbolTable& sym, LocalScope& vars, llvmBuilder& bldr, Module& modl, AstBlock& body);

void genIf(LLVMContext& ctx, SymbolTable& sym, LocalScope& scope, llvmBuilder& bldr, Module& modl, AstIf& node)
{
    Value* condition = genExpr(ctx, sym, node.condition, scope, bldr);
    BasicBlock* ifBlock = BasicBlock::Create(ctx, "", bldr.GetInsertBlock()->getParent());
    BasicBlock* postBlock = BasicBlock::Create(ctx, "", bldr.GetInsertBlock()->getParent());
    bldr.CreateCondBr(condition, ifBlock, postBlock);
    bldr.SetInsertPoint(ifBlock);
    // #TODO: generate a local scope within the block?
    genBlock(ctx, sym, scope, bldr, modl, node.block);
    bldr.CreateBr(postBlock);
    bldr.SetInsertPoint(postBlock);
}

void genStatement(LLVMContext& ctx, SymbolTable& sym, LocalScope& scope, llvmBuilder& bldr, Module& modl, NodePtr& node)
{
    switch (node.type)
    {
    case NodeType::Declaration:
        genLocalVariableDecl(ctx, sym, scope, *static_cast<AstDecl*>(node.data), bldr);
        return;
    case NodeType::Assignment:
        genAssign(ctx, sym, scope, bldr, *static_cast<AstAssign*>(node.data));
        return;
    case NodeType::Call:
        genCall(ctx, sym, scope, bldr, modl, *static_cast<AstCall*>(node.data));
        return;
    case NodeType::If:
        genIf(ctx, sym, scope, bldr, modl, *static_cast<AstIf*>(node.data));
        return;
    }

    std::cout << "cant handle: " << static_cast<u32>(node.type) << '\n';
    exit(-1);
}

// #INCOMPLETE: return values ?
void genBlock(LLVMContext& ctx, SymbolTable& sym, LocalScope& vars, llvmBuilder& bldr, Module& modl, AstBlock& body)
{
    for (NodePtr& node : body.statements)
    {
        genStatement(ctx, sym, vars, bldr, modl, node);
    }
}

Function* genExternalFn(LLVMContext& ctx, llvmBuilder& bldr, Module& modl, AstFnInterface& fn)
{
    return genFnInterface(ctx, fn, modl);
}

Function* genFn(LLVMContext& ctx, SymbolTable& sym, AstFn& node, Module& modl, legacy::FunctionPassManager& fpm)
{
    Function* fn = modl.getFunction(node.iface.name);

    BasicBlock* body = BasicBlock::Create(ctx, "", fn);
    IRBuilder<> builder{ body, body->begin() };

    LocalScope variables;
    u32 i = 0;
    for (auto& arg : fn->args())
    {
        // Can have more args than interface parameters if fn is main (implicit params)
        if (i >= node.iface.parameters.size())
            break;

        variables.put(node.iface.parameters[i].identifier, VarType::Param, &arg);
        i++;
    }

    genBlock(ctx, sym, variables, builder, modl, node.block);
    builder.CreateRetVoid();
    fpm.run(*fn);

    return fn;
}

void createModule(ParseCtx& ast)
{
    LLVMContext ctx{};
    Module modl{ "fname", ctx };
    legacy::FunctionPassManager fpm{ &modl };

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto triple = sys::getDefaultTargetTriple();
    std::string err;
    auto target = TargetRegistry::lookupTarget(triple, err);
    if (!target)
    {
        std::cout << "failed to get LLVM target\n";
        exit(-1);
    }

    auto cpu = "generic";
    auto features = "";
    TargetOptions opt;
    auto machine = target->createTargetMachine(triple, cpu, features, opt, Reloc::PIC_);

    fpm.add(createPromoteMemoryToRegisterPass());
    fpm.add(createInstructionCombiningPass());
    fpm.add(createReassociatePass());
    
    std::error_code errc;
    raw_fd_ostream dest("silver.o", errc);

    legacy::PassManager pm{};
    if (machine->addPassesToEmitFile(pm, dest, nullptr, CodeGenFileType::CGFT_ObjectFile))
    {
        errs() << "Target and file type incompatible";
        exit(-1);
    }

    modl.setDataLayout(machine->createDataLayout());
    modl.setTargetTriple(triple);

    for (AstFn& fn : ast.functions)
        genFnInterface(ctx, fn.iface, modl);
    for (AstFn& fn : ast.functions)
        genFn(ctx, ast.symbols, fn, modl, fpm);

    modl.print(errs(), nullptr);
    pm.run(modl);
}
