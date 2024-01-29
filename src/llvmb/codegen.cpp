#include "codegen.hpp"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/raw_ostream.h"

// #TODO: investigate llvm/Support/VirtualFileSystem.h


using namespace llvm;

LocalScope::LocalScope()
{
    sh_new_arena(hashTable);
}

void LocalScope::put(const char* name, AllocaInst * inst)
{
    shput(hashTable, name, inst);
}

AllocaInst* LocalScope::get(const char* name)
{
    i64 idx = shgeti(hashTable, name);
    if (idx == -1)
    {
        std::cout << "scope lookup fail wahh\n";
    }

    return hashTable[idx].value;
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

Value* genVarExpr(LLVMContext& ctx, AstVarExpr& node, LocalScope& scope, llvmBuilder& bldr)
{
    return scope.get(node.identifier);
}

Value* genLoad(LLVMContext& ctx, AstVarExpr* node, LocalScope& scope, llvmBuilder& bldr)
{
    AllocaInst* var = scope.get(node->identifier);
    return bldr.CreateLoad(var->getAllocatedType(), var, node->identifier);
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

    FunctionType* fntype = FunctionType::get(resolveType(ctx, node.returnType), false);
    Function* fn = Function::Create(fntype, Function::ExternalLinkage, Twine(node.name), modl);
    return fn;
}

Value* genExpr(LLVMContext& ctx, NodePtr& node, LocalScope& scope, llvmBuilder& bldr)
{
    switch (node.type)
    {
    case NodeType::Integer:
        return genInteger(ctx, static_cast<AstInteger*>(node.data));
    case NodeType::VarExpr:
        return genLoad(ctx, static_cast<AstVarExpr*>(node.data), scope, bldr);
    case NodeType::String:
        return genStringLiteral(ctx, static_cast<AstString*>(node.data), scope, bldr);
    }

    std::cout << "rahhh " << static_cast<u32>(node.type) << "\n";
    exit(-1);
}

// Generates an alloca at function scope, and a possible store at local scope
AllocaInst* genLocalVariableDecl(LLVMContext& ctx, LocalScope& scope, AstDecl& decl, llvmBuilder& bldr)
{
    Function* fn = bldr.GetInsertBlock()->getParent();
    IRBuilder<> entry{ &fn->getEntryBlock(), fn->getEntryBlock().begin() };
    AllocaInst* inst = entry.CreateAlloca(resolveType(ctx, decl.type), nullptr, decl.identifier);
    scope.put(decl.identifier, inst);

    if (decl.valueExpr.data != nullptr)
    {
        bldr.CreateStore(genExpr(ctx, decl.valueExpr, scope, bldr), inst);
    }
    return inst;
}

void genAssign(LLVMContext& ctx, LocalScope& scope, llvmBuilder& bldr, AstAssign& asgn)
{
    Value* val = genExpr(ctx, asgn.expr, scope, bldr);
    Value* lhs = genVarExpr(ctx, asgn.lhs, scope, bldr);
    bldr.CreateStore(val, lhs);
}

Value* genCall(LLVMContext& ctx, LocalScope& scope, llvmBuilder& bldr, Module& modl, AstCall& call)
{
    // #BAD: alloc
    Array<Value*> args{ u32(call.args.size()) };
    Array<Type*> argTys{ u32(call.args.size()) };
    argTys[0] = Type::getInt8PtrTy(ctx);
    for (u32 i = 0; i < call.args.size(); i++)
    {
        args[i] = genExpr(ctx, call.args[i], scope, bldr);
    }
    // #TODO: type resolution on all exprs and variables
    FunctionType* callTy = FunctionType::get(Type::getInt32Ty(ctx), ArrayRef<Type*>{ argTys.begin(), argTys.end()}, false);
    FunctionCallee target = modl.getOrInsertFunction(call.name, callTy);
    return bldr.CreateCall(target, ArrayRef<Value*>{ args.begin(), args.end() });
}

void genStatement(LLVMContext& ctx, LocalScope& scope, llvmBuilder& bldr, Module& modl, NodePtr& node)
{
    switch (node.type)
    {
    case NodeType::Declaration:
        genLocalVariableDecl(ctx, scope, *static_cast<AstDecl*>(node.data), bldr);
        return;
    case NodeType::Assignment:
        genAssign(ctx, scope, bldr, *static_cast<AstAssign*>(node.data));
        return;
    case NodeType::Call:
        genCall(ctx, scope, bldr, modl, *static_cast<AstCall*>(node.data));
        return;
    }

    complain("Don't know how to handle node", 0);
}

// #INCOMPLETE: return values ?
void genFnBody(LLVMContext& ctx, LocalScope& vars, llvmBuilder& bldr, Module& modl, AstBlock& body)
{
    for (NodePtr& node : body.statements)
    {
        genStatement(ctx, vars, bldr, modl, node);
    }
}

Function* genExternalFn(LLVMContext& ctx, llvmBuilder& bldr, Module& modl, AstFnInterface& fn)
{
    return genFnInterface(ctx, fn, modl);
}

Function* genFn(LLVMContext& ctx, AstFn& node, Module& modl, legacy::FunctionPassManager& fpm)
{
    Function* fn = genFnInterface(ctx, node.iface, modl);

    BasicBlock* body = BasicBlock::Create(ctx, "", fn);
    IRBuilder<> builder{ body, body->begin() };

    LocalScope variables;
    genFnBody(ctx, variables, builder, modl, node.block);

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
        genFn(ctx, fn, modl, fpm);

    modl.print(errs(), nullptr);
    pm.run(modl);
}