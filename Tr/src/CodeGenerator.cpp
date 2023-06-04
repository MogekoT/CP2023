/*
 ***************** CodeGenerator.h ******************
 * This file implements the CodeGenerator class,	*
 * and initialize the global variables needed for	*
 * code generation.									*
 ****************************************************
 */

#include "CodeGenerator.hpp"

llvm::LLVMContext Context;

llvm::IRBuilder<> IRBuilder(Context);

CodeGenerator::CodeGenerator(void) :
	Module(new llvm::Module("main", Context)),
	DL(new llvm::DataLayout(Module)),
	CurrFunction(NULL),
	StructTyTable(NULL),
	SymbolTableStack(),
	ContinueBlockStack(),
	BreakBlockStack(),
	TmpBB(NULL),
	TmpFunc(NULL)
{}

void CodeGenerator::PushSymbolTable(void) {
	SymbolTable* newSymbolTable = new SymbolTable;
	SymbolTableStack.push_back(newSymbolTable);
}

void CodeGenerator::PopSymbolTable(void) {
	if (isEmptySymbolTableStack()) return;
	else {
		SymbolTable* lastSymbolTable = SymbolTableStack.back();
		SymbolTableStack.pop_back();
		delete lastSymbolTable;
	}
}

llvm::TypeSize CodeGenerator::GetTypeSize(llvm::Type* Type) {
	return this->DL->getTypeAllocSize(Type);
}

llvm::Function* CodeGenerator::FindFunction(std::string Name) {
	if (isEmptySymbolTableStack()) return NULL;
	else {
		for (auto i = this->SymbolTableStack.end() - 1; i >= this->SymbolTableStack.begin(); i--) {
			auto p = (**i).find(Name);
			if (p != (**i).end()) return p->second.GetFunction();
		}
		return NULL;
	}
}

llvm::Type* CodeGenerator::FindType(std::string Name) {
	if (isEmptySymbolTableStack()) return NULL;
	else {
		for (auto i = this->SymbolTableStack.end() - 1; i >= this->SymbolTableStack.begin(); i--) {
			auto p = (**i).find(Name);
			if (p != (**i).end()) return p->second.GetType();
		}
		return NULL;
	}
}

llvm::Value* CodeGenerator::FindVariable(std::string Name) {
	if (isEmptySymbolTableStack()) return NULL;
	else {
		for (auto i = this->SymbolTableStack.end() - 1; i >= this->SymbolTableStack.begin(); i--) {
			auto p = (**i).find(Name);
			if (p != (**i).end()) return p->second.GetVariable();
		}
		return NULL;
	}
}

llvm::Value* CodeGenerator::FindConstant(std::string Name) {
	if (isEmptySymbolTableStack()) return NULL;
	else {
		for (auto i = this->SymbolTableStack.end() - 1; i >= this->SymbolTableStack.begin(); i--) {
			auto p = (**i).find(Name);
			if (p != (**i).end()) return p->second.GetConstant();
		}
		return NULL;
	}
}

llvm::BasicBlock* CodeGenerator::GetContinueBlock(void) {
	if (this->ContinueBlockStack.size()) return this->ContinueBlockStack.back();
	else return NULL;
}

llvm::BasicBlock* CodeGenerator::GetBreakBlock(void) {
	if (this->BreakBlockStack.size()) return this->BreakBlockStack.back();
	else return NULL;
}

llvm::Function* CodeGenerator::GetCurrentFunction(void) {
	return this->CurrFunction;
}

bool isEmptySymbolTableStack(void) {
	if (this->SymbolTableStack.empty()) return true;
	else return false;
}

bool CodeGenerator::AddFunction(std::string Name, llvm::Function* Function) {
	if (isEmptySymbolTableStack()) return false;
	else {
		auto& t = *(this->SymbolTableStack.back());
		auto p = t.find(Name);
		if (p != t.end()) return false;
		t[Name] = Symbol(Function);
		return true;
	}
}

bool CodeGenerator::AddType(std::string Name, llvm::Type* Type) {
	if (isEmptySymbolTableStack()) return false;
	else {
		auto& t = *(this->SymbolTableStack.back());
		auto p = t.find(Name);
		if (p != t.end()) return false;
		else {
			t[Name] = Symbol(Type);
			return true;
		}
	}
}

bool CodeGenerator::AddVariable(std::string Name, llvm::Value* Variable) {
	if (isEmptySymbolTableStack()) return false;
	else {
		auto& t = *(this->SymbolTableStack.back());
		auto p = t.find(Name);
		if (p != t.end()) return false;
		else {
			t[Name] = Symbol(Variable, false);
			return true;
		}
	}
}

bool CodeGenerator::AddConstant(std::string Name, llvm::Value* Constant) {
	if (isEmptySymbolTableStack()) return false;
	else {
		auto& t = *(this->SymbolTableStack.back());
		auto p = t.find(Name);
		if (p != t.end()) return false;
		else {
			t[Name] = Symbol(Constant, true);
			return true;
		}
	}	
}

bool CodeGenerator::AddStructType(llvm::StructType* Ty1, AST::StructType* Ty2) {
	auto p = this->StructTyTable->find(Ty1);
	if (p != this->StructTyTable->end()) return false;
	else {
		(*this->StructTyTable)[Ty1] = Ty2;
		return true;
	}

}

void CodeGenerator::EnterFunction(llvm::Function* Func) {
	this->CurrFunction = Func;
}

void CodeGenerator::LeaveFunction(void) {
	this->CurrFunction = NULL;
}

void CodeGenerator::EnterLoop(llvm::BasicBlock* ContinueBB, llvm::BasicBlock* BreakBB) {
	this->ContinueBlockStack.push_back(ContinueBB);
	this->BreakBlockStack.push_back(BreakBB);
}

void CodeGenerator::LeaveLoop(void) {
	if (!(this->ContinueBlockStack.size()) || !(this->BreakBlockStack.size())) return;
	else {
		this->ContinueBlockStack.pop_back();
		this->BreakBlockStack.pop_back();
	}

}

void CodeGenerator::XchgInsertPointWithTmpBB(void) {
	auto Tmp = IRBuilder.GetInsertBlock();
	IRBuilder.SetInsertPoint(this->TmpBB);
	this->TmpBB = Tmp;
}

void CodeGenerator::GenerateCode(AST::Program& Root, const std::string& OptimizeLevel) {

	this->StructTyTable = new StructTypeTable;
	this->PushSymbolTable();

	this->TmpFunc = llvm::Function::Create(llvm::FunctionType::get(IRBuilder.getVoidTy(), false), llvm::GlobalValue::InternalLinkage, "0Tmp", this->Module);
	this->TmpBB = llvm::BasicBlock::Create(Context, "Temp", this->TmpFunc);

	Root.CodeGen(*this);
	std::cout << "Gen Successfully" << std::endl;

	this->TmpBB->eraseFromParent();
	this->TmpFunc->eraseFromParent();

	this->PopSymbolTable();
	delete this->StructTyTable;
	this->StructTyTable = NULL;
}

void CodeGenerator::DumpIRCode(std::string FileName) {
    if (FileName.empty()) {
		FileName = "-";
	}
    std::error_code EC;
    llvm::raw_fd_ostream Out(FileName, EC);
    Out << "-----------------  IRCode  ----------------\n";
    this->Module->print(Out, NULL);
    Out << "----------------  Verification  --------------\n";
    if (!llvm::verifyModule(*this->Module, &Out)) {
        Out << "No errors.\n";
    }
}

void CodeGenerator::GenObjectCode(std::string FileName) {
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    std::string Error;
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        throw std::runtime_error("Failed to get target: " + Error);
        return;
    }
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, "", "", opt, RM);
    if (!TargetMachine) {
        throw std::runtime_error("Failed to create target machine");
        return;
    }
    Module->setDataLayout(TargetMachine->createDataLayout());
    Module->setTargetTriple(TargetTriple);
    std::error_code EC;
    llvm::raw_fd_ostream Dest(FileName, EC, llvm::sys::fs::OF_None);
    if (EC) {
        throw std::runtime_error("Could not open file: " + EC.message());
        return;
    }
    llvm::legacy::PassManager PM;
    auto FileType = llvm::CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(PM, Dest, nullptr, FileType)) {
        throw std::runtime_error("TargetMachine can't emit a file of this type");
        return;
    }
    PM.run(*Module);
    Dest.flush();
}

void CodeGenerator::GenHTML(std::string FileName, AST::Program& Root) {
	extern const char* Html;
	std::string OutputString = Html;
	std::string Json = Root.astJson();
	std::string Target = "${ASTJson}";
	auto Pos = OutputString.find(Target);
	OutputString.replace(Pos, Target.length(), Json.c_str());
	std::ofstream HTMLFile(FileName);
	HTMLFile << OutputString;
}

AST::StructType* CodeGenerator::FindStructType(llvm::StructType* Ty1) {
	auto p = this->StructTyTable->find(Ty1);
	if (p != this->StructTyTable->end()) return p->second;
	else return NULL;
}