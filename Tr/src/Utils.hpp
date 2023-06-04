#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include "CodeGenerator.hpp"
//Cast
llvm::Value* CastIntToBool(llvm::Value* Value, llvm::IRBuilder<> &IRBuilder) {
    return IRBuilder.CreateICmpNE(Value, llvm::ConstantInt::get((llvm::IntegerType*)Value->getType(), 0, true));
}

llvm::Value* CastFloatToBool(llvm::Value* Value, llvm::IRBuilder<> &IRBuilder) {
    return IRBuilder.CreateFCmpONE(Value, llvm::ConstantFP::get(Value->getType(), 0.0));
}

llvm::Value* CastPointerToBool(llvm::Value* Value, llvm::IRBuilder<> &IRBuilder) {
    return IRBuilder.CreateICmpNE(IRBuilder.CreatePtrToInt(Value, IRBuilder.getInt64Ty()), IRBuilder.getInt64(0));
}

llvm::Value* Cast(llvm::Value* Value, llvm::IRBuilder<> &IRBuilder) {
    if (Value->getType() == IRBuilder.getInt1Ty())
        return Value;
    else if (Value->getType()->isIntegerTy())
        return CastIntToBool(Value, IRBuilder);
    else if (Value->getType()->isFloatingPointTy())
        return CastFloatToBool(Value, IRBuilder);
    else if (Value->getType()->isPointerTy())
        return CastPointerToBool(Value, IRBuilder);
    else {
        throw std::logic_error("Cannot cast to bool type.");
        return NULL;
    }
}

// TypeCasting
llvm::Value* CastIntToInt(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreateIntCast(Value, Type, !Value->getType()->isIntegerTy(1));
}

llvm::Value* CastIntToFloat(llvm::Value* Value, llvm::Type* Type) {
    return Value->getType()->isIntegerTy(1) ?
        IRBuilder.CreateUIToFP(Value, Type) : IRBuilder.CreateSIToFP(Value, Type);
}

llvm::Value* CastIntToPointer(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreateIntToPtr(Value, Type);
}

llvm::Value* CastFloatToInt(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreateFPToSI(Value, Type);
}

llvm::Value* CastFloatToFloat(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreateFPCast(Value, Type);
}

llvm::Value* CastPointerToInt(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreatePtrToInt(Value, Type);
}

llvm::Value* CastPointerToPointer(llvm::Value* Value, llvm::Type* Type) {
    return IRBuilder.CreatePointerCast(Value, Type);
}

llvm::Value* TypeCasting(llvm::Value* Value, llvm::Type* Type) {
    if (Value->getType() == Type) {
        return Value;
    }
    else if (Type == IRBuilder.getInt1Ty()) {    //Int1 (bool) is special.
        return Cast(Value);
    }
    else if (Value->getType()->isIntegerTy() && Type->isIntegerTy()) {
        return CastIntToInt(Value, Type);
    }
    else if (Value->getType()->isIntegerTy() && Type->isFloatingPointTy()) {
        return CastIntToFloat(Value, Type);
    }
    else if (Value->getType()->isIntegerTy() && Type->isPointerTy()) {
        return CastIntToPointer(Value, Type);
    }
    else if (Value->getType()->isFloatingPointTy() && Type->isIntegerTy()) {
        return CastFloatToInt(Value, Type);
    }
    else if (Value->getType()->isFloatingPointTy() && Type->isFloatingPointTy()) {
        return CastFloatToFloat(Value, Type);
    }
    else if (Value->getType()->isPointerTy() && Type->isIntegerTy()) {
        return CastPointerToInt(Value, Type);
    }
    else if (Value->getType()->isPointerTy() && Type->isPointerTy()) {
        return CastPointerToPointer(Value, Type);
    }
    else {
        return NULL;
    }
}

//Create a left/right shift llvm::Value
llvm::Value* CreateShift(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator, bool isLeftShift) {
	if (!(LHS->getType()->isIntegerTy() && RHS->getType()->isIntegerTy())) {
		throw std::domain_error("Shifting operator must be applied to 2 integers.");
		return NULL;
	}
	TypeUpgrading(LHS, RHS);
	if (isLeftShift) {
		return IRBuilder.CreateShl(LHS, RHS);
	} else {
		return IRBuilder.CreateAShr(LHS, RHS);
	}
}

llvm::Value* CreateShl(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	return CreateShift(LHS, RHS, Generator, true);
}

llvm::Value* CreateShr(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	return CreateShift(LHS, RHS, Generator, false);
}

llvm::Value* CreateICmp(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator, llvm::CmpInst::Predicate IntPredicate, llvm::CmpInst::Predicate FloatPredicate) {
	if (TypeUpgrading(LHS, RHS)) {
		if (LHS->getType()->isIntegerTy())
			return IRBuilder.CreateICmp(IntPredicate, LHS, RHS);
		else
			return IRBuilder.CreateFCmp(FloatPredicate, LHS, RHS);
	}
	throw std::domain_error("Comparison using unsupported type combination.");
}
/*&*/
llvm::Value* CreateBitwiseAND(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (!(LHS->getType()->isIntegerTy() && RHS->getType()->isIntegerTy())) {
		throw std::domain_error("Bitwise AND operator \"&\" must be applied to 2 integers.");
		return NULL;
	}
	TypeUpgrading(LHS, RHS);
	return IRBuilder.CreateAnd(LHS, RHS);
}

/*|*/
llvm::Value* CreateBitwiseOR(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (!(LHS->getType()->isIntegerTy() && RHS->getType()->isIntegerTy())) {
		throw std::domain_error("Bitwise OR operator \"|\" must be applied to 2 integers.");
		return NULL;
	}
	TypeUpgrading(LHS, RHS);
	return IRBuilder.CreateOr(LHS, RHS);
}

/* + - * / % */
llvm::Value* CreateArithmetic(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator, llvm::Instruction::BinaryOps IntOp, llvm::Instruction::BinaryOps FloatOp) {
	if (TypeUpgrading(LHS, RHS)) {
		if (LHS->getType()->isIntegerTy())
			return IRBuilder.CreateBinOp(IntOp, LHS, RHS);
		else
			return IRBuilder.CreateBinOp(FloatOp, LHS, RHS);
	}
	throw std::logic_error("Arithmetic using unsupported type combination.");
	return NULL;
}

llvm::Value* CreateAdd(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
		auto TMP = LHS;
		LHS = RHS;
		RHS = TMP;
	}
	if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
		return IRBuilder.CreateGEP(LHS->getType()->getNonOpaquePointerElementType(), LHS, RHS);
	}
	return CreateArithmetic(LHS, RHS, Generator, llvm::Instruction::Add, llvm::Instruction::FAdd);
}

llvm::Value* CreateSub(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
		return IRBuilder.CreateGEP(LHS->getType()->getNonOpaquePointerElementType(), LHS, IRBuilder.CreateNeg(RHS));
	}
	if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType())
		return IRBuilder.CreatePtrDiff(LHS->getType()->getNonOpaquePointerElementType(), LHS, RHS);
	return CreateArithmetic(LHS, RHS, Generator, llvm::Instruction::Sub, llvm::Instruction::FSub);
}

llvm::Value* CreateMul(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	return CreateArithmetic(LHS, RHS, Generator, llvm::Instruction::Mul, llvm::Instruction::FMul);
}

llvm::Value* CreateDiv(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	return CreateArithmetic(LHS, RHS, Generator, llvm::Instruction::SDiv, llvm::Instruction::FDiv);
}

llvm::Value* CreateMod(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (!(LHS->getType()->isIntegerTy() && RHS->getType()->isIntegerTy())) {
		throw std::domain_error("Modulo operator \"%\" must be applied to 2 integers.");
		return NULL;
	}
	TypeUpgrading(LHS, RHS);
	return IRBuilder.CreateSRem(LHS, RHS);
}
llvm::Value* CreateAssignment(llvm::Value* pLHS, llvm::Value* RHS, CodeGenerator& Generator) {
	RHS = TypeCasting(RHS, pLHS->getType()->getNonOpaquePointerElementType());
	if (RHS == NULL) {
		throw std::domain_error("Assignment with values that cannot be cast to the target type.");
		return NULL;
	}
	IRBuilder.CreateStore(RHS, pLHS);
	return pLHS;
}
llvm::Value* CreateLoad(llvm::Value* pLHS, CodeGenerator& Generator) {
	if (pLHS->getType()->getNonOpaquePointerElementType()->isArrayTy())
		return IRBuilder.CreatePointerCast(pLHS, pLHS->getType()->getNonOpaquePointerElementType()->getArrayElementType()->getPointerTo());
	else
		return IRBuilder.CreateLoad(pLHS->getType()->getNonOpaquePointerElementType(), pLHS);
}

llvm::Value* TypeUpgrading(llvm::Value* Value, llvm::Type* Type) {
    auto ValueType = Value->getType();
    if (ValueType->isIntegerTy() && Type->isIntegerTy()) {
        size_t BitValue = ((llvm::IntegerType*)ValueType)->getBitWidth();
        size_t BitType = ((llvm::IntegerType*)Type)->getBitWidth();
        if (BitValue < BitType)
            return IRBuilder.CreateIntCast(Value, Type, BitValue != 1);
        else return Value;
    }
    else if (ValueType->isFloatingPointTy() && Type->isFloatingPointTy()) {
        if (ValueType->isFloatTy() && Type->isDoubleTy())
            return IRBuilder.CreateFPCast(Value, Type);
        else return Value;
    }
    else if (ValueType->isIntegerTy() && Type->isFloatingPointTy()) {
        return ValueType->isIntegerTy(1) ?
            IRBuilder.CreateUIToFP(Value, Type) : IRBuilder.CreateSIToFP(Value, Type);
    }
    else if (ValueType->isFloatingPointTy() && Type->isIntegerTy()) {
        return Value;
    }
    else return NULL;
}
bool TypeUpgrading(llvm::Value*& Value1, llvm::Value*& Value2) {
    auto Type1 = Value1->getType();
    auto Type2 = Value2->getType();
    if (Type1->isIntegerTy() && Type2->isIntegerTy()) {
        size_t Bit1 = ((llvm::IntegerType*)Type1)->getBitWidth();
        size_t Bit2 = ((llvm::IntegerType*)Type2)->getBitWidth();
        if (Bit1 < Bit2)
            Value1 = IRBuilder.CreateIntCast(Value1, Type2, Bit1 != 1);
        else if (Bit1 > Bit2)
            Value2 = IRBuilder.CreateIntCast(Value2, Type1, Bit2 != 1);
        return true;
    }
    else if (Type1->isFloatingPointTy() && Type2->isFloatingPointTy()) {
        if (Type1->isFloatTy() && Type2->isDoubleTy())
            Value1 = IRBuilder.CreateFPCast(Value1, IRBuilder.getDoubleTy());
        else if (Type1->isDoubleTy() && Type2->isFloatTy())
            Value2 = IRBuilder.CreateFPCast(Value2, IRBuilder.getDoubleTy());
        return true;
    }
    else if (Type1->isIntegerTy() && Type2->isFloatingPointTy()) {
        Value1 = Type1->isIntegerTy(1) ?
            IRBuilder.CreateUIToFP(Value1, Type2) : IRBuilder.CreateSIToFP(Value1, Type2);
        return true;
    }
    else if (Type1->isFloatingPointTy() && Type2->isIntegerTy()) {
        Value2 = Type2->isIntegerTy(1) ?
            IRBuilder.CreateUIToFP(Value2, Type1) : IRBuilder.CreateSIToFP(Value2, Type1);
        return true;
    }
    else return false;
}

llvm::Value* checkCondition(llvm::Value* Condition) {
    if (Condition == NULL) {
        throw std::logic_error("The first operand of thernary operand \" ? : \" must be able to be cast to boolean.");
        return NULL;
    }
    return Condition;
}

llvm::Value* findVariableOrConstant(CodeGenerator& Generator, const std::string& name) {
    llvm::Value* VarPtr = Generator.FindVariable(name);
    if (VarPtr) return VarPtr;
    VarPtr = Generator.FindConstant(name);
    if (VarPtr) return VarPtr;
    throw std::logic_error("Identifier \"" + name + "\" is neither a variable nor a constant.");
    return NULL;
}
llvm::Value* createICmpNE(llvm::Value* LHS, llvm::Value* RHS) {
    if (LHS->getType()->isIntegerTy())
        return IRBuilder.CreateICmpNE(LHS, RHS);
    else
        return IRBuilder.CreateFCmpONE(LHS, RHS);
}
llvm::BranchInst* TerminateBlockByBr(llvm::BasicBlock* BB) {
	//If not terminated, jump to the target block
	if (!IRBuilder.GetInsertBlock()->getTerminator())
		return IRBuilder.CreateBr(BB);
	else
		return NULL;
}
llvm::Value* CreateAssignment(llvm::Value* pLHS, llvm::Value* RHS, CodeGenerator& Generator) {
	RHS = TypeCasting(RHS, pLHS->getType()->getNonOpaquePointerElementType());
	if (RHS == NULL) {
		throw std::domain_error("Assignment with values that cannot be cast to the target type.");
		return NULL;
	}
	IRBuilder.CreateStore(RHS, pLHS);
	return pLHS;
}
llvm::Value* CreateBitwiseXOR(llvm::Value* LHS, llvm::Value* RHS, CodeGenerator& Generator) {
	if (!(LHS->getType()->isIntegerTy() && RHS->getType()->isIntegerTy())) {
		throw std::domain_error("Bitwise XOR operator \"^\" must be applied to 2 integers.");
		return NULL;
	}
	TypeUpgrading(LHS, RHS);
	return IRBuilder.CreateXor(LHS, RHS);
}
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* Func, std::string VarName, llvm::Type* VarType) {
	llvm::IRBuilder<> TmpB(&Func->getEntryBlock(), Func->getEntryBlock().begin());
	return TmpB.CreateAlloca(VarType, 0, VarName);
}
//Create an equal-comparison instruction. This function will automatically do type casting
llvm::Value* CreateCmpEQ(llvm::Value* LHS, llvm::Value* RHS) {
	//Arithmatic compare
	if (TypeUpgrading(LHS, RHS)) {
		if (LHS->getType()->isIntegerTy())
			return IRBuilder.CreateICmpEQ(LHS, RHS);
		else
			return IRBuilder.CreateFCmpOEQ(LHS, RHS);
	}
	//Pointer compare
	if (LHS->getType()->isPointerTy() && LHS->getType() == RHS->getType()) {
		return IRBuilder.CreateICmpEQ(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isPointerTy() && RHS->getType()->isIntegerTy()) {
		return IRBuilder.CreateICmpEQ(
			IRBuilder.CreatePtrToInt(LHS, IRBuilder.getInt64Ty()),
			TypeUpgrading(RHS, IRBuilder.getInt64Ty())
		);
	}
	else if (LHS->getType()->isIntegerTy() && RHS->getType()->isPointerTy()) {
		return IRBuilder.CreateICmpEQ(
			TypeUpgrading(LHS, IRBuilder.getInt64Ty()),
			IRBuilder.CreatePtrToInt(RHS, IRBuilder.getInt64Ty())
		);
	}
	throw std::domain_error("Comparison \"==\" using unsupported type combination.");
	return NULL;
}
