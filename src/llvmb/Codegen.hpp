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


#include "src/ast.hpp"


#pragma once

// A collection of function-local variables
#include "stb_ds.h"
#include "lib.hpp"

#include "llvm/IR/IRBuilder.h"

#include <iostream>

enum class VarType : u8
{
    LocalVar,
    Param
};

class LocalScope
{
public:
    struct Entry
    {
        const char* key;
        VarType type;
        llvm::Value* val;
    };
    LocalScope();

    void put(const char* name, VarType type, llvm::Value* val);

    Entry* get(const char* name);

    ~LocalScope();
private:
    Entry* hashTable;
};

using llvmBuilder = llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>;

void createModule(ParseCtx& ast);