#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"


#include "parse/Ast.hpp"
#include "llvmScope.hpp"

using namespace llvm;

using llvmBuilder = IRBuilder<ConstantFolder, IRBuilderDefaultInserter>;

Type* resolveType(LLVMContext& ctx, String& name)
{
	return Type::getInt32Ty(ctx);
}

Value* genInteger(LLVMContext& ctx, AstInteger* node)
{
	return ConstantInt::get(Type::getInt32Ty(ctx), node->value);
}

Value* genVarExpr(LLVMContext& ctx, AstVarExpr& node, llvmScope& scope, llvmBuilder& bldr)
{
	return scope.get(node.identifier);
}

Value* genLoad(LLVMContext& ctx, AstVarExpr* node, llvmScope& scope, llvmBuilder& bldr)
{
	AllocaInst* var = scope.get(node->identifier);
	return bldr.CreateLoad(var->getAllocatedType(), var, node->identifier.data());
}

Function* genFnInterface(LLVMContext& ctx, AstFnInterface& node, Module& modl)
{
	FunctionType* fntype = FunctionType::get(Type::getVoidTy(ctx), false);
	Function* fn = Function::Create(fntype, Function::ExternalLinkage, Twine(node.name.data()), modl);
	/// TODO: above deprecated?
	return fn;
}

Value* genExpr(LLVMContext& ctx, NodePtr& node, llvmScope& scope, llvmBuilder& bldr)
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
AllocaInst* genLocalVariableDecl(LLVMContext& ctx, llvmScope& scope, AstDecl& decl, llvmBuilder& bldr)
{
	Function* fn = bldr.GetInsertBlock()->getParent();
	IRBuilder<> entry{ &fn->getEntryBlock(), fn->getEntryBlock().begin() };
	AllocaInst* inst = entry.CreateAlloca(resolveType(ctx, decl.type), nullptr, decl.identifier.data());
	scope.put(decl.identifier, inst);

	if (decl.valueExpr.data != nullptr)
	{
		bldr.CreateStore(genExpr(ctx, decl.valueExpr, scope, bldr), inst);
	}
	return inst;
}

void genAssign(LLVMContext& ctx, llvmScope& scope, llvmBuilder& bldr, AstAssign& asgn)
{
	Value* val = genExpr(ctx, asgn.expr, scope, bldr);
	Value* lhs = genVarExpr(ctx, asgn.lhs, scope, bldr);
	bldr.CreateStore(val, lhs);
}

Value* genCall(LLVMContext& ctx, llvmScope& scope, llvmBuilder& bldr, Module& modl, AstCall& call)
{
	// #BAD: alloc
	Array<Value*> args{ call.args.size() };
	for (u32 i = 0; i < call.args.size(); i++)
	{
		args[i] = genExpr(ctx, call.args[i], scope, bldr);
	}
	// #TODO: type resolution on all exprs and variables
	Type* argTys[] = { Type::getInt8PtrTy(ctx) };
	FunctionType* callTy = FunctionType::get(Type::getInt32Ty(ctx), argTys);
	FunctionCallee target = modl.getOrInsertFunction(call.name.data(), callTy);
	return nullptr;
	// return bldr.CreateCall(target, ArrayRef{ args.begin(), args.end() });
}

void genStatement(LLVMContext& ctx, llvmScope& scope, llvmBuilder& bldr, Module& modl, NodePtr& node)
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
void genFnBody(LLVMContext& ctx, llvmScope& vars, llvmBuilder& bldr, Module& modl, AstBlock& body)
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

	llvmScope variables;
	genFnBody(ctx, variables, builder, modl, node.block);

	builder.CreateRet(variables.get(String("x", 3)));
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
