#include "CodeGenerator.hpp"
#include "Utils.hpp"

//function declartion

//Error # 新加入的，可能不需要
    llvm::Value* LogError(const char *Str) {
        std::cout<< Str <<endl;
        return NULL;
    }

namespace AST{
    //Program: several declarations # 没必要动，就这样吧
    llvm::Value* Program::CodeGen(CodeGenerator& Generator) {
		for (auto Decl : *(this->m_Decls))
			if (Decl)
				Decl->CodeGen(Generator);
		return NULL;
	}
llvm::Value* FuncDecl::CodeGen(CodeGenerator& Generator) {
    std::vector<llvm::Type*> ArgTypes;
    for (auto ArgType : *(this->m_ArgList)) {
        llvm::Type* LLVMType = ArgType->m_VarType->GetLLVMType(Generator);
        if (LLVMType->isArrayTy())
            LLVMType = LLVMType->getArrayElementType()->getPointerTo();
        ArgTypes.push_back(LLVMType);
    }
    llvm::Type* RetTy = this->m_RetType->GetLLVMType(Generator);
    llvm::FunctionType* FuncType = llvm::FunctionType::get(RetTy, ArgTypes, this->m_ArgList->m_VarArg);
    llvm::Function* Func = llvm::Function::Create(FuncType, llvm::GlobalValue::ExternalLinkage, this->m_Name, Generator.Module);
    Generator.AddFunction(this->m_Name, Func);
    if (Func->getName() != this->m_Name) {
        Func->eraseFromParent();
        Func = Generator.Module->getFunction(this->m_Name);
    }
    if (this->_FuncBody) {
        llvm::BasicBlock* FuncBlock = llvm::BasicBlock::Create(Context, "entry", Func);
        IRBuilder.SetInsertPoint(FuncBlock);
        Generator.PushSymbolTable();
        size_t Index = 0;
        for (auto ArgIter = Func->arg_begin(); ArgIter < Func->arg_end(); ArgIter++, Index++) {
            auto Alloc = CreateEntryBlockAlloca(Func, this->m_ArgList->at(Index)->m_Name, ArgTypes[Index]);
            IRBuilder.CreateStore(ArgIter, Alloc);
            Generator.AddVariable(this->m_ArgList->at(Index)->m_Name, Alloc);
        }
        Generator.EnterFunction(Func);
        Generator.PushSymbolTable();
        this->_FuncBody->CodeGen(Generator);
        Generator.PopSymbolTable();
        Generator.LeaveFunction();
        Generator.PopSymbolTable();
    }
    return NULL;
}
//Function body
llvm::Value* FuncBody::CodeGen(CodeGenerator& Generator) {
		//Generate the statements in FuncBody, one by one.
		for (auto& Stmt : *(this->_Content))
			//If the current block already has a terminator,
			//i.e. a "return" statement is generated, stop;
			//Otherwise, continue generating.
			if (IRBuilder.GetInsertBlock()->getTerminator())
				break;
			else
				Stmt->CodeGen(Generator);
		//If the function doesn't have a "return" at the end of its body, create a default one.
		if (!IRBuilder.GetInsertBlock()->getTerminator()) {
			llvm::Type* RetTy = Generator.GetCurrentFunction()->getReturnType();
			if (RetTy->isVoidTy())
				IRBuilder.CreateRetVoid();
			else
				IRBuilder.CreateRet(llvm::UndefValue::get(RetTy));
		}
		return NULL;
	}
//    &&
llvm::Value* LogicAND::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	LHS = Cast(LHS);
	if (LHS == NULL) {
		throw std::domain_error("Logic AND operator \"&&\" must be applied to 2 expressions that can be cast to boolean.");
		return NULL;
	}
	RHS = Cast(RHS);
	if (RHS == NULL) {
		throw std::domain_error("Logic AND operator \"&&\" must be applied to 2 expressions that can be cast to boolean.");
		return NULL;
	}
	return IRBuilder.CreateLogicalAnd(LHS, RHS);
}
llvm::Value* LogicAND::CodeGenPtr(CodeGenerator& Generator) {
		throw std::logic_error("Logic AND operator \"&&\" only returns right-values.");
		return NULL;
	}
//    ||
llvm::Value* LogicOR::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	LHS = Cast(LHS);
	if (LHS == NULL) {
		throw std::domain_error("Logic OR operator \"||\" must be applied to 2 expressions that can be cast to boolean.");
		return NULL;
	}
	RHS = Cast(RHS);
	if (RHS == NULL) {
		throw std::domain_error("Logic OR operator \"||\" must be applied to 2 expressions that can be cast to boolean.");
		return NULL;
	}
		return IRBuilder.CreateLogicalOr(LHS, RHS);
	}
llvm::Value* LogicOR::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Logic OR operator \"||\" only returns right-values.");
	return NULL;
}
//    &
llvm::Value* BitwiseAND::CodeGen(CodeGenerator& Generator) {
		llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
		llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
		return CreateBitwiseAND(LHS, RHS, Generator);
	}
llvm::Value* BitwiseAND::CodeGenPtr(CodeGenerator& Generator) {
		throw std::logic_error("Bitwise AND operator \"&\" only returns right-values.");
		return NULL;
	}
//    | 
llvm::Value* BitwiseOR::CodeGen(CodeGenerator& Generator) {
		llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
		llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
		return CreateBitwiseOR(LHS, RHS, Generator);
	}
llvm::Value* BitwiseOR::CodeGenPtr(CodeGenerator& Generator) {
		throw std::logic_error("Bitwise OR operator \"|\" only returns right-values.");
		return NULL;
	}
//    ^ 
llvm::Value* BitwiseXOR::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateBitwiseXOR(LHS, RHS, Generator);
}
llvm::Value* BitwiseXOR::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Bitwise XOR operator \"^\" only returns right-values.");
	return NULL;
}
//    ~
llvm::Value* BitwiseNot::CodeGen(CodeGenerator& Generator) {
	llvm::Value* Operand = this->_Operand->CodeGen(Generator);
	if (!Operand->getType()->isIntegerTy()) {
		throw std::logic_error("Bitwise NOT operator \"~\" must be applied to integers.");
		return NULL;
	}
	return IRBuilder.CreateNot(Operand);
}
llvm::Value* BitwiseNot::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Bitwise NOT operator \"~\" only returns right-values.");
	return NULL;
}
//x+y
llvm::Value* Addition::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAdd(LHS, RHS, Generator);
}
llvm::Value* Addition::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Addition operator \"+\" only returns right-values.");
	return NULL;
}
//x/y
llvm::Value* Division::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateDiv(LHS, RHS, Generator);
}
llvm::Value* Division::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Division operator \"/\" only returns right-values.");
	return NULL;
}

//x*y
llvm::Value* Multiplication::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateMul(LHS, RHS, Generator);
}
llvm::Value* Multiplication::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Multiplication operator \"*\" only returns right-values.");
	return NULL;
}
//x+y
llvm::Value* Addition::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAdd(LHS, RHS, Generator);
}
llvm::Value* Addition::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Addition operator \"+\" only returns right-values.");
	return NULL;
}

//   x-y
llvm::Value* Subtraction::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateSub(LHS, RHS, Generator);
}
llvm::Value* Subtraction::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Subtraction operator \"-\" only returns right-values.");
	return NULL;
}
//   x%y
llvm::Value* Modulo::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateMod(LHS, RHS, Generator);
}
llvm::Value* Modulo::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Modulo operator \"%\" only returns right-values.");
	return NULL;
}
//   x*=y
llvm::Value* MulAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* MulAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateMul(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}
//   x%=y
llvm::Value* ModAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* ModAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateMod(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}
//   x+=y
llvm::Value* AddAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* AddAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateAdd(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}
//   x-=y
llvm::Value* SubAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* SubAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateSub(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}
//   x*=y
llvm::Value* MulAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* MulAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateMul(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}
//   x/=y
llvm::Value* DivAssign::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* DivAssign::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGenPtr(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateAssignment(LHS,
		CreateDiv(
			IRBuilder.CreateLoad(
				LHS->getType()->getNonOpaquePointerElementType(),
				LHS),
			RHS,
			Generator),
		Generator
	);
}

//    =
llvm::Value* DirectAssign::CodeGen(CodeGenerator& Generator) {
		return CreateLoad(this->CodeGenPtr(Generator), Generator);
	}
llvm::Value* DirectAssign::CodeGenPtr(CodeGenerator& Generator) {
		llvm::Value* LHS = this->_LHS->CodeGenPtr(Generator);
		llvm::Value* RHS = this->_RHS->CodeGen(Generator);
		return CreateAssignment(LHS, RHS, Generator);
	}
//    x>=y
llvm::Value* logicGRE::CodeGen(CodeGenerator& Generator) {
    llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
    llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
    //Arithmatic compare
    if (TypeUpgrading(LHS, RHS)) {
        if (LHS->getType()->isIntegerTy())
            return IRBuilder.CreateICmpSGE(LHS, RHS);
        else
            return IRBuilder.CreateFCmpOGE(LHS, RHS);
    }
    //Pointer compare
    if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
        return IRBuilder.CreateICmpUGE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
        return IRBuilder.CreateICmpUGE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            TypeUpgrading(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
        return IRBuilder.CreateICmpUGE(
            TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    throw std::domain_error("Comparison \">=\" using unsupported type combination.");
}
llvm::Value* logicGRE::CodeGenPtr(CodeGenerator& Generator) {
    throw std::logic_error("Comparison operator \">=\" only returns right-values.");
    return NULL;
}

//    x<=y
llvm::Value* logicSME::CodeGen(CodeGenerator& Generator) {
    llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
    llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
    //Arithmatic compare
    if (TypeUpgrading(LHS, RHS)) {
        if (LHS->getType()->isIntegerTy())
            return IRBuilder.CreateICmpSLE(LHS, RHS);
        else
            return IRBuilder.CreateFCmpOLE(LHS, RHS);
    }
    //Pointer compare
    if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
        return IRBuilder.CreateICmpULE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
        return IRBuilder.CreateICmpULE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            TypeUpgrading(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
        return IRBuilder.CreateICmpULE(
            TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    throw std::domain_error("Comparison \"<=\" using unsupported type combination.");
}
llvm::Value* logicSME::CodeGenPtr(CodeGenerator& Generator) {
    throw std::logic_error("Comparison operator \"<=\" only returns right-values.");
    return NULL;
}
//    x>y
llvm::Value* logicGR::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	//Arithmatic compare
	if (TypeUpgrading(LHS, RHS)) {
		if (LHS->getType()->isIntegerTy())
			return IRBuilder.CreateICmpSGT(LHS, RHS);
		else
			return IRBuilder.CreateFCmpOGT(LHS, RHS);
	}
	//Pointer compare
	if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
		return IRBuilder.CreateICmpUGT(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
		return IRBuilder.CreateICmpUGT(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			TypeUpgrading(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
		return IRBuilder.CreateICmpUGT(
			TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	throw std::domain_error("Comparison \">\" using unsupported type combination.");
}
	llvm::Value* logicGR::CodeGenPtr(CodeGenerator& Generator) {
		throw std::logic_error("Comparison operator \">\" only returns right-values.");
		return NULL;
	}
//    x<y
llvm::Value* logicSM::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	//Arithmatic compare
	if (TypeUpgrading(LHS, RHS)) {
		if (LHS->getType()->isIntegerTy())
			return IRBuilder.CreateICmpSLT(LHS, RHS);
		else
			return IRBuilder.CreateFCmpOLT(LHS, RHS);
	}
	//Pointer compare
	if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
		return IRBuilder.CreateICmpULT(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
		return IRBuilder.CreateICmpULT(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			TypeUpgrading(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
		return IRBuilder.CreateICmpULT(
			TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	throw std::domain_error("Comparison \"<\" using unsupported type combination.");
}
llvm::Value* logicSM::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Comparison operator \"<\" only returns right-values.");
	return NULL;
}
//    == 
llvm::Value* LogicEQ::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateCmpEQ(LHS, RHS);
}
llvm::Value* LogicEQ::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Comparison operator \"==\" only returns right-values.");
	return NULL;
}
//   !=*/
llvm::Value* LogicNEQ::CodeGen(CodeGenerator& Generator) {
    llvm::Value* LHS = this->_LHS->CodeGen(Generator);
    llvm::Value* RHS = this->_RHS->CodeGen(Generator);
    //Arithmatic compare
    if (TypeUpgrading(LHS, RHS)) {
        return createICmpNE(LHS, RHS);
    }
    //Pointer compare
    if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
        return IRBuilder.CreateICmpNE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
        return IRBuilder.CreateICmpNE(
            IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
            TypeUpgrading(RHS, IRBuilder.getInt64Ty())
        );
    }
    else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
        return IRBuilder.CreateICmpNE(
            TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
            IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
        );
    }
    throw std::domain_error("Comparison \"!=\" using unsupported type combination.");
}

llvm::Value* LogicNEQ::CodeGenPtr(CodeGenerator& Generator) {
    throw std::logic_error("Comparison operator \"!=\" only returns right-values.");
    return NULL;
}
//   !*/
llvm::Value* LogicNot::CodeGen(CodeGenerator& Generator) {
	return IRBuilder.CreateICmpEQ(Cast(this->_Operand->CodeGen(Generator)), IRBuilder.getInt1(false));
}
llvm::Value* LogicNot::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Logic NOT operator \"!\" only returns right-values.");
	return NULL;
}
//   << 
llvm::Value* LeftShift::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateShl(LHS, RHS, Generator);
}
llvm::Value* LeftShift::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Left shifting operator \"<<\" only returns right-values.");
	return NULL;
}
 /* >> */
llvm::Value* RightShift::CodeGen(CodeGenerator& Generator) {
	llvm::Value* LHS = this->m_LHS->CodeGen(Generator);
	llvm::Value* RHS = this->m_RHS->CodeGen(Generator);
	return CreateShr(LHS, RHS, Generator);
}
llvm::Value* RightShift::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Right shifting operator \">>\" only returns right-values.");
	return NULL;
}
class TernaryCondition : public Opcode {  /* ?*/
//Comma
llvm::Value* CommaExpr::CodeGen(CodeGenerator& Generator) {
	this->m_LHS->CodeGen(Generator);
	return this->m_RHS->CodeGen(Generator);
}

//?x:y
llvm::Value* TernaryCondition::CodeGen(CodeGenerator& Generator) {
    llvm::Value* Condition = checkCondition(Cast(this->m_Condition->CodeGen(Generator)));
    llvm::Value* True = this->m_Then->CodeGen(Generator);
    llvm::Value* False = this->m_Else->CodeGen(Generator);
    if (True->getType() == False->getType() || TypeUpgrading(True, False)) {
        return IRBuilder.CreateSelect(Condition, True, False);
    }
    else {
        throw std::domain_error("Thernary operand \" ? : \" using unsupported type combination.");
        return NULL;
    }
}
llvm::Value* TernaryCondition::CodeGenPtr(CodeGenerator& Generator) {
    llvm::Value* Condition = checkCondition(Cast(this->m_Condition->CodeGen(Generator)));
    llvm::Value* True = this->m_Then->CodeGenPtr(Generator);
    llvm::Value* False = this->m_Else->CodeGenPtr(Generator);
    if (True->getType() != False->getType()) {
        throw std::domain_error("When using thernary expressions \" ? : \" as left-values, the latter two operands must be of the same type.");
        return NULL;
    }
    return IRBuilder.CreateSelect(Condition, True, False);
}
llvm::Value* Constant::CodeGen(CodeGenerator& Generator) {
	switch (this->_Type) {
	case BuiltInType::TypeID::m_Bool:
		return IRBuilder.getInt1(this->m_Bool);
	case BuiltInType::TypeID::m_Char:
		return IRBuilder.getInt8(this->m_Character);
	case BuiltInType::TypeID::m_Int:
		return IRBuilder.getInt32(this->m_Integer);
	case BuiltInType::TypeID::m_Double:
		return llvm::ConstantFP::get(IRBuilder.getDoubleTy(), this->m_Real);
	}
}
llvm::Value* Constant::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Constant is a right-value.");
	return NULL;
}
class GlobalString;
class TypeCast;
llvm::Value* Variable::CodeGen(CodeGenerator& Generator) {
    llvm::Value* VarPtr = findVariableOrConstant(Generator, this->m_Name);
    if (VarPtr == Generator.FindConstant(this->m_Name)) return VarPtr;
    return CreateLoad(VarPtr, Generator);
}
llvm::Value* Variable::CodeGenPtr(CodeGenerator& Generator) {
    llvm::Value* VarPtr = findVariableOrConstant(Generator, this->m_Name);
    if (VarPtr == Generator.FindConstant(this->m_Name)) {
        throw std::logic_error("\"" + this->m_Name + "\" is an immutable constant, not a left-value.");
        return NULL;
    }
    return VarPtr;
}

llvm::Value* Indirection::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* Indirection::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* Ptr = this->_Operand->CodeGen(Generator);
	if (!Ptr->getType()->isPointerTy()) {
		throw std::logic_error("Indirection operator \"*\" only applies on pointers or arrays.");
		return NULL;
	}
	return Ptr;
}
llvm::Value* StructReference::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* StructReference::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* StructPtr = this->_Struct->CodeGenPtr(Generator);
	if (!StructPtr->getType()->isPointerTy() || !StructPtr->getType()->getNonOpaquePointerElementType()->isStructTy()) {
		throw std::logic_error("Reference operator \".\" must be apply to structs or unions.");
		return NULL;
	}
	//Since C language uses name instead of index to fetch the element inside a struct,
	//we need to fetch the AST::StructType* instance according to the llvm::StructType* instance.
	//And it is the same with union types.
	AST::StructType* StructType = Generator.FindStructType((llvm::StructType*)StructPtr->getType()->getNonOpaquePointerElementType());
	if (StructType) {
		int MemIndex = StructType->GetElementIndex(this->m_MemName);
		if (MemIndex == -1) {
			throw std::logic_error("The struct doesn't have a member whose name is \"" + this->m_MemName + "\".");
			return NULL;
		}
		std::vector<llvm::Value*> Indices;
		Indices.push_back(IRBuilder.getInt32(0));
		Indices.push_back(IRBuilder.getInt32(MemIndex));
		return IRBuilder.CreateGEP(StructPtr->getType()->getNonOpaquePointerElementType(), StructPtr, Indices);
	}
}
//   a->x
llvm::Value* StructDereference::CodeGen(CodeGenerator& Generator) {
	return CreateLoad(this->CodeGenPtr(Generator), Generator);
}
llvm::Value* StructDereference::CodeGenPtr(CodeGenerator& Generator) {
	llvm::Value* StructPtr = this->_StructPtr->CodeGen(Generator);
	if (!StructPtr->getType()->isPointerTy() || !StructPtr->getType()->getNonOpaquePointerElementType()->isStructTy()) {
		throw std::logic_error("Dereference operator \"->\" must be apply to struct or union pointers.");
		return NULL;
	}
		//Since C language uses name instead of index to fetch the element inside a struct,
		//we need to fetch the AST::StructType* instance according to the llvm::StructType* instance.
		//And it is the same with union types.
	AST::StructType* StructType = Generator.FindStructType((llvm::StructType*)StructPtr->getType()->getNonOpaquePointerElementType());
	if (StructType) {
		int MemIndex = StructType->GetElementIndex(this->m_MemName);
		if (MemIndex == -1) {
			throw std::logic_error("The struct doesn't have a member whose name is \"" + this->m_MemName + "\".");
			return NULL;
		}
		std::vector<llvm::Value*> Indices;
		Indices.push_back(IRBuilder.getInt32(0));
		Indices.push_back(IRBuilder.getInt32(MemIndex));
		return IRBuilder.CreateGEP(StructPtr->getType()->getNonOpaquePointerElementType(), StructPtr, Indices);
	}
}
//   &
llvm::Value* AddressOf::CodeGen(CodeGenerator& Generator) {
		return this->m_Operand->CodeGenPtr(Generator);
	}
llvm::Value* AddressOf::CodeGenPtr(CodeGenerator& Generator) {
	throw std::logic_error("Address operator \"&\" only returns right-values.");
	return NULL;
}
    //Variable declaration # 修改变量名，删除重写部分无用的逻辑
    llvm::Value* VarDecl::CodeGen(CodeGenerator& Generator) {
		llvm::Type* Var_Type; 
        Var_Type = this->m_VarType->GetLLVMType(Generator);
        
		if (!Generator.GetCurrentFunction()) {
            llvm::Constant* Init = NULL;
            Generator.XchgInsertPointWithTmpBB();

            llvm:Value* InitExpr = TypeCasting(IRBuilder.getInt32(10), Var_Type);
            if (InitExpr == NULL) {
                throw std::logic_error("Initializing variable with value of different type.");
				return NULL;
            }
            Generator.XchgInsertPointWithTmpBB();
            Init = (llvm::Constant*)InitExpr;

            auto Alloc = new llvm::GlobalVariable (
                *(Generator.Module),
                Var_Type,
                false,
                llvm::Function::ExternalLinkage,
                Init,
                ID
            );

            if (!Generator.AddVariable(ID, Alloc)) {
                throw std::logic_error("Redefining global variable " + ID+ ".");
		        Alloc->eraseFromParent();
		        return NULL;
            }
        }
        else {
            auto Alloc = CreateEntryBlockAlloca(Generator.GetCurrentFunction(), ID, Var_Type);
			if (!Generator.AddVariable(ID, Alloc)) {
					throw std::logic_error("Redefining local variable " + ID + ".");
					Alloc->eraseFromParent();
					return NULL;
				}
        }

        return NULL;
	}

    //Type declaration # 修改变量名，删除重写部分无用的逻辑
    llvm::Value* TypeDecl::CodeGen(CodeGenerator& Generator) {
		llvm::Type* LLVMType;

        if (!Generator.AddType(this->Rename, LLVMType))
			throw std::logic_error("Redefinition of typename " + this->Rename);

		if (this->Var_Type->isStructType())
			LLVMType = ((AST::StructType*)this->Var_Type)->GenerateLLVMTypeHead(Generator, this->Rename);
            ((AST::StructType*)this->Var_Type)->GenerateLLVMTypeBody(Generator);
        else if (this->Var_Type->isUnionType())
			LLVMType = ((AST::UnionType*)this->Var_Type)->GenerateLLVMTypeHead(Generator, this->Rename);
            ((AST::UnionType*)this->Var_Type)->GenerateLLVMTypeBody(Generator);
        else
			LLVMType = this->Var_Type->GetLLVMType(Generator);
		if (!LLVMType)
			throw std::logic_error("Typedef " + this->Rename + " using undefined types.");

		return NULL;
	}

    //Built-in type # 删除部分不需要类型，修改内部逻辑
    llvm::Type* BuiltInType::GetLLVMType(CodeGenerator& Generator) {
        static llvm:Type* BuiltInType;
		if (this->m_LLVMType)
			return this->m_LLVMType;
        if (this->m_Type == m_Bool)
            this->m_LLVMType = IRBuilder.getInt1Ty();
        if (this->m_Type == m_Int)
            this->m_LLVMType = IRBuilder.getInt32Ty();
        if (this->m_Type == m_Double)
            this->m_LLVMType = IRBuilder.getDoubleTy();
        if (this->m_Type == m_Char)
            this->m_LLVMType = IRBuilder.getInt8Ty();
        if (this->m_Type == m_Void)
            this->m_LLVMType = IRBuilder.getVoidTy();
		return this->m_LLVMType;
	}

    //Struct type # 改了变量名，需要继续改
    llvm::Type* StructType::GetLLVMType(CodeGenerator& Generator) {
		if (this->m_LLVMType)
			return this->m_LLVMType;
		this->GenerateLLVMTypeHead(Generator);
		return this->GenerateLLVMTypeBody(Generator);
	}
	llvm::Type* StructType::GenerateLLVMTypeHead(CodeGenerator& Generator, const std::string& Name) {
		auto LLVMType = llvm::StructType::create(Context, "struct." + Name);
		Generator.AddStructType(LLVMType, this);
		return this->m_LLVMType = LLVMType;
	}
	llvm::Type* StructType::GenerateLLVMTypeBody(CodeGenerator& Generator) {
		std::vector<llvm::Type*> Members;
		for (auto FDecl : *(this->m_StructBody))
			if (FDecl->m_VarType->GetLLVMType(Generator)->isVoidTy()) {
				throw std::logic_error("The member type of struct cannot be void.");
				return NULL;
			}
			else
				Members.insert(Members.end(), FDecl->m_MemList->size(), FDecl->m_VarType->GetLLVMType(Generator));
		((llvm::StructType*)this->m_LLVMType)->setBody(Members);
		return this->m_LLVMType;
	}
	size_type StructType::GetElementIndex(const std::string& MemName) {
		size_type Index = 0;
		for (auto FDecl : *(this->m_StructBody))
			for (auto& MemName : *(FDecl->m_MemList))
				if (MemName == MemName)
					return Index;
				else Index++;
		return -1;
	}

	//Pointer type # 感觉没必要动,不知道怎么改
    llvm::Type* PointerType::GetLLVMType(CodeGenerator& Generator) {
		if (this->m_LLVMType)
			return this->m_LLVMType;
		llvm::Type* BaseType = this->m_BaseType->GetLLVMType(Generator);
		return this->m_LLVMType = llvm::PointerType::get(BaseType, 0U);
	}

	//If statement # 删除部分注释，更改部分变量名
    llvm::Value* Stmt_If::CodeGen(CodeGenerator& Generator) {
		llvm::Value* Condition = this->m_Condition->CodeGen(Generator);

		llvm::Function* CurrentFunc = Generator.GetCurrentFunction();
		llvm::BasicBlock* If_Then = llvm::BasicBlock::Create(Context, "Then");
		llvm::BasicBlock* If_Else = llvm::BasicBlock::Create(Context, "Else");
		llvm::BasicBlock* If_Merge = llvm::BasicBlock::Create(Context, "Merge");

		IRBuilder.CreateCondBr(Condition, If_Then, If_Else);
		CurrentFunc->getBasicBlockList().push_back(If_Then);
		IRBuilder.SetInsertPoint(If_Then);
		if (this->m_Then) {
			Generator.PushSymbolTable();
			this->m_Then->CodeGen(Generator);
			Generator.PopSymbolTable();
		}
		TerminateBlockByBr(If_Merge);
		CurrentFunc->getBasicBlockList().push_back(If_Else);
		IRBuilder.SetInsertPoint(If_Else);
		if (this->m_Else) {
			Generator.PushSymbolTable();
			this->m_Else->CodeGen(Generator);
			Generator.PopSymbolTable();
		}
		TerminateBlockByBr(If_Merge);
		if (If_Merge->hasNPredecessorsOrMore(1)) {
			CurrentFunc->getBasicBlockList().push_back(If_Merge);
			IRBuilder.SetInsertPoint(If_Merge);
		}
		return NULL;
	}

    //While statement # 删除部分注释，更改部分变量名
    llvm::Value* Stmt_While::CodeGen(CodeGenerator& Generator) {
		llvm::Function* CurrentFunc = Generator.GetCurrentFunction();
		llvm::BasicBlock* While_Condition = llvm::BasicBlock::Create(Context, "WhileCond");
		llvm::BasicBlock* While_Loop = llvm::BasicBlock::Create(Context, "WhileLoop");
		llvm::BasicBlock* While_End = llvm::BasicBlock::Create(Context, "WhileEnd");
		IRBuilder.CreateBr(While_Condition);
		
        //Condition
		CurrentFunc->getBasicBlockList().push_back(While_Condition);
		IRBuilder.SetInsertPoint(While_Condition);
		llvm::Value* Condition = this->m_Condition->CodeGen(Generator);
		IRBuilder.CreateCondBr(Condition, While_Loop, While_End);

		//Loop
		CurrentFunc->getBasicBlockList().push_back(While_Loop);
		IRBuilder.SetInsertPoint(While_Loop);
		if (this->m_LoopBody) {
			Generator.EnterLoop(While_Condition, While_End);
			Generator.PushSymbolTable();
			this->m_LoopBody->CodeGen(Generator);
			Generator.PopSymbolTable();
			Generator.LeaveLoop();
		}
		TerminateBlockByBr(While_Condition);
		
        //End
		CurrentFunc->getBasicBlockList().push_back(While_End);
		IRBuilder.SetInsertPoint(While_End);
		return NULL;
	}

    //Break statement # 改完了，微调了一下逻辑顺序，不改了
    llvm::Value* Stmt_Break::CodeGen(CodeGenerator& Generator) {
		llvm::BasicBlock* my_Break = Generator.GetBreakBlock();
		if (!my_Break)
			throw std::logic_error("Break statement should only be used in loops or switch statements.");
        else
		    IRBuilder.CreateBr(my_Break);
		return NULL;
	}

    //Return statement # 改完了，调整逻辑顺序，但是理论上只为了跑test可以删除部分
    llvm::Value* Stmt_Return::CodeGen(CodeGenerator& Generator) {
		llvm::Function* my_Function = Generator.GetCurrentFunction();
		if (this->m_ReturnValue != NULL) {
            llvm::Value* ReturnValue = TypeCasting(this->m_ReturnValue->CodeGen(Generator), my_Function->getReturnType());
			if (!ReturnValue) {
				throw std::logic_error("The type of return value doesn't match and cannot be cast to the return type.");
				return NULL;
			}
			IRBuilder.CreateRet(ReturnValue);
		}
		else if(this->m_ReturnValue == NULL){
			if (my_Function->getReturnType()->isVoidTy())
				IRBuilder.CreateRetVoid();
			else {
				throw std::logic_error("Expected expression after return statement.");
				return NULL;
			}
		}
	}

}

