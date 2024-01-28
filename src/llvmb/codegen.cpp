#include "codegen.hpp"

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
        Type::getVoidTy(ctx);
    }
    
    std::cout << "cant handle this type\n";
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

Function* genFnInterface(LLVMContext& ctx, AstFnInterface& node, Module& modl)
{
    FunctionType* fntype = FunctionType::get(resolveType(ctx, node.returnType), false);
    Function* fn = Function::Create(fntype, Function::ExternalLinkage, Twine(node.name), modl);
    /// TODO: above deprecated?
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
    }

    std::cout << "rahhh " << static_cast<u32>(node.type) << "\n";
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
    FunctionType* callTy = FunctionType::get(Type::getInt32Ty(ctx), ArrayRef{ argTys.begin(), argTys.end()}, false);
    FunctionCallee target = modl.getOrInsertFunction(call.name, callTy);
    return nullptr;
    return bldr.CreateCall(target, ArrayRef{ args.begin(), args.end() });
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

    fpm.add(createPromoteMemoryToRegisterPass());
    fpm.add(createInstructionCombiningPass());
    fpm.add(createReassociatePass());

    for (AstFn& fn : ast.functions)
        genFn(ctx, fn, modl, fpm);

    modl.print(errs(), nullptr);
}