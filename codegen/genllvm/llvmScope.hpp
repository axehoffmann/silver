#pragma once

// A collection of function-local variables

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "lib.hpp"

#include "llvm/IR/IRBuilder.h"

#include <iostream>

class llvmScope
{
private:
	struct Entry
	{
		char* key;
		llvm::AllocaInst* value;
	};
public:
	llvmScope()
	{
		sh_new_arena(hashTable);
	}

	void put(String& name, llvm::AllocaInst* inst)
	{
		shput(hashTable, name.data(), inst);
	}

	llvm::AllocaInst* get(const String& name)
	{
		i64 idx = shgeti(hashTable, name.data());
		if (idx == -1)
		{
			std::cout << "scope lookup fail wahh\n";
		}

		return hashTable[idx].value;
	}

	~llvmScope()
	{
		shfree(hashTable);
	}

private:
	Entry* hashTable;
};