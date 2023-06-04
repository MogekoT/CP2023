#pragma once
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include <exception>

#include "AST.hpp"

extern llvm::LLVMContext Context;

extern llvm::IRBuilder<> IRBuilder;

class CodeGenerator {
public:
	llvm::Module* Module;
public:

	CodeGenerator(void);

	void PushSymbolTable(void);

	void PopSymbolTable(void);

	llvm::TypeSize GetTypeSize(llvm::Type* Type);

	llvm::Function* FindFunction(std::string Name);

	llvm::Value* FindVariable(std::string Name);

	llvm::Value* FindConstant(std::string Name);

	llvm::Function* GetCurrentFunction(void);

	llvm::Type* FindType(std::string Name);

	llvm::BasicBlock* GetContinueBlock(void);

	llvm::BasicBlock* GetBreakBlock(void);

	bool AddFunction(std::string Name, llvm::Function* Function);

	bool AddType(std::string Name, llvm::Type* Type);

	bool AddVariable(std::string Name, llvm::Value* Variable);

	bool AddConstant(std::string Name, llvm::Value* Constant);

	bool AddStructType(llvm::StructType* Ty1, AST::StructType* Ty2);

	void EnterFunction(llvm::Function* Func);

	void LeaveFunction(void);
	
	void EnterLoop(llvm::BasicBlock* ContinueBB, llvm::BasicBlock* BreakBB);

	void LeaveLoop(void);

	void XchgInsertPointWithTmpBB(void);

	void GenerateCode(AST::Program& Root, const std::string& OptimizeLevel = "");

	void DumpIRCode(std::string FileName);

	void GenObjectCode(std::string FileName);

	void GenHTML(std::string FileName, AST::Program& Root);

	AST::StructType* FindStructType(llvm::StructType* Ty1);

private:

	using StructTypeTable = std::map<llvm::StructType*, AST::StructType*>;

	class Symbol {
	public:
		Symbol(void) : Content(NULL), Type(UNDEFINED) {}
		Symbol(llvm::Function* Func) : Content(Func), Type(FUNCTION) {}
		Symbol(llvm::Type* Ty) : Content(Ty), Type(TYPE) {}
		Symbol(llvm::Value* Value, bool isConst) : Content(Value), Type(isConst ? CONSTANT : VARIABLE) {}
		llvm::Function* GetFunction(void) { 
			if (this->Type == FUNCTION) return (llvm::Function*)Content;
			else NULL; 
		}
		llvm::Type* GetType(void) { 
			if (this->Type == TYPE) return (llvm::Type*)Content;
			else NULL; 
		}
		llvm::Value* GetVariable(void) { 
			if (this->Type == VARIABLE) return (llvm::Value*)Content;
			else NULL; 
		}
		llvm::Value* GetConstant(void) { 
			if (this->Type == CONSTANT) return (llvm::Value*)Content;
			else NULL; 
		}

	private:
		void* Content;
		enum {
			FUNCTION,
			TYPE,
			VARIABLE,
			CONSTANT,
			UNDEFINED
		} Type;
	};

	using SymbolTable = std::map<std::string, Symbol>;

private:
	llvm::DataLayout* DL;
	llvm::Function* CurrFunction;
	StructTypeTable* StructTyTable;
	std::vector<SymbolTable*> SymbolTableStack;
	std::vector<llvm::BasicBlock*> ContinueBlockStack;
	std::vector<llvm::BasicBlock*> BreakBlockStack;
	llvm::BasicBlock* TmpBB;
	llvm::Function* TmpFunc;

private:
	bool isEmptySymbolTableStack(void);
};