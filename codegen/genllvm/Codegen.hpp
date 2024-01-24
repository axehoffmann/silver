#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"

#include "parse/Ast.hpp"

using namespace llvm;

Value* genInteger(LLVMContext& ctx, AstInteger* node)
{
	return ConstantInt::get(Type::getInt32Ty(ctx), node->value);
}

Function* genFnInterface(LLVMContext& ctx, AstFn* node)
{
	FunctionType* fntype = FunctionType::get(Type::getVoidTy(ctx), false);
	Function* fn = Function::Create(fntype, Function::ExternalLinkage, Twine(node->name.data()));
	/// TODO: above deprecated?
	return fn;
}

Function* genFn(LLVMContext& ctx, IRBuilder<LLVMContext>& builder, AstFn* node)
{
	Function* fn = genFnInterface(ctx, node);
	
	BasicBlock* body = BasicBlock::Create(ctx, "", fn);
	builder.SetInsertPoint(body);
	Module* mod;
}
