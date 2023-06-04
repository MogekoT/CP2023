#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include <exception>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
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

class CodeGenerator;

namespace AST{
  
  //Node
  class Node;

  //Root
  class Program;

  //Declarations
    class Decl;
    using Decls = std::vector<Decl*>;
		  class FuncDecl;
			  class Arg;
			  class ArgList;
			  class FuncBody;
		  class VarDecl;
			  class VarInit;
			  using VarList = std::vector<VarInit*>;
		  class TypeDecl;
    
  //Variable Types
    class VarType;
      class DefinedType;
		  class PointerType;
		  class StructType;
  //Expression
	class Expr;
		class Constant;
			class GlobalString;
		class Variable;
		class OpCode;
			class LogicAND;
			class LogicOR;
			class LogicNot;
			class LogicGT;
			class LogicGE;
			class LogicLT;
			class LogicLE;
			class LogicEQ;
			class LogicNEQ;
			class BitwiseAND;
			class BitwiseXOR;
			class BitwiseOR;
			class DirectAssign;
			class DivAssign;
			class MulAssign;
			class ModAssign;
			class AddAssign;
			class SubAssign;
			class Division;
			class Multiplication;
			class Modulo;
			class Addition;
			class Subtraction;
			class Subscript;
			class BitwiseNot;
			class LeftShift;
			class RightShift;
			class TernaryCondition;
			class TypeCast;
			class StructReference;
			class StructDereference;
			class Indirection;
			class AddressOf;
			class CommaExpr;

  //Statements
    class Stmt;
	  using Stmts = std::vector<Stmt*>;
		  class Stmt_If;
		  class Stmt_While;
		  class Stmt_Break;
		  class Stmt_Return;

}		
	
		

// Class definitions
namespace AST{

  //AST Node # 没必要动，删除析构函数
  	class Node {
	public:
		Node(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
	};

  //Root: Program # 没必要动，删除析构函数
  	class Program : public Node {
	public:
		Decls* m_Decls;
		Program(Decls* m_Decls) :m_Decls(m_Decls) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};

  //Statement # 没必要动，删除析构函数
  	class Stmt : public Node {
	public:
		Stmt(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
	};

  //Declaration # 把public Stmt换成了public Node，删除析构函数
 	 class Decl : public Node {
	public:
		Decl(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
	};

  //Variable declaration
  	class VarDecl : public Decl {
	public:
		VarType* m_VarType;
		VarList* m_VarList;

		VarDecl(VarType* VarType, VarList* VarList) :
			m_VarType(VarType), m_VarList(VarList) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};

  //Pointer type
  	class PointerType : public VarType {
	public:
		VarType* m_BaseType;

		PointerType(VarType* BaseType) : m_BaseType(BaseType) {}

		llvm::Type* GetLLVMType(CodeGenerator& Generator);

		bool isBuiltInType(void) { return false; }
		bool isDefinedType(void) { return false; }
		bool isPointerType(void) { return true; }
		bool isStructType(void) { return false; }
		std::string astJson();
	};

  //Struct Type
	class StructType : public VarType {
	public:

		FieldDecls* m_StructBody;

		StructType(FieldDecls* StructBody) : m_StructBody(StructBody) {}

		llvm::Type* GetLLVMType(CodeGenerator& Generator);

		llvm::Type* GenerateLLVMTypeHead(CodeGenerator& Generator, const std::string& Name = "<unnamed>");
		llvm::Type* GenerateLLVMTypeBody(CodeGenerator& Generator);

		size_t GetElementIndex(const std::string& MemName);

		bool isBuiltInType(void) { return false; }
		bool isDefinedType(void) { return false; }
		bool isPointerType(void) { return false; }
		bool isStructType(void) { return true; }
		std::string astJson();
	};

  //Built-in type
   	class BuiltInType : public VarType {
	public:
		//Enum of built-in type
		enum TypeID {
			m_Bool,
			m_int,
			m_Char,
			m_Double,
			m_Void
		};
		//Type ID
		TypeID _Type;

		BuiltInType(TypeID Type) : m_Type(Type) {}

		llvm::Type* GetLLVMType(CodeGenerator& Generator);
		bool isBuiltInType(void) { return true; }
		bool isDefinedType(void) { return false; }
		bool isPointerType(void) { return false; }
		bool isStructType(void) { return false; }
		std::string astJson();
	};

  //If statement
	class Stmt_If : public Stmt {
	public:
		//Branch condition, then-clause and else-clause
		Expr* m_Condition;
		Stmt* m_Then;
		Stmt* m_Else;

		Stmt_If(Expr* Condition, Stmt* Then, Stmt* Else = NULL) : m_Condition(Condition), m_Then(Then), m_Else(Else) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};

  //While statement
	class Stmt_While : public Stmt {
	public:
		//Loop condition and loop body
		Expr* m_Condition;
		Stmt* m_LoopBody;

		Stmt_While(Expr* Condition, Stmt* LoopBody) : m_Condition(Condition), m_LoopBody(LoopBody) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};

	//Break statement
	class Stmt_Break : public Stmt {
	public:
		Stmt_Break(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};

	//Return statement
	class Stmt_Return : public Stmt {
	public:
		//The expression to be returned (if any)
		Expr* m_RetVal;
		Stmt_Return(Expr* RetVal = NULL) : m_RetVal(RetVal) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		std::string astJson();
	};


class Node {
	public:
		Node(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
	};
/*Expression*/
class Expr : public Node {
	public:
		Expr(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual llvm::Value* CodeGenPtr(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
	};
class Constant : public Expr {
	public:
		BuiltInType::TypeID m_Type;
		bool m_Bool;
		char m_Character;
		int m_Integer;
		double m_Real;
		Constant(bool Bool) :
			m_Type(BuiltInType::TypeID::m_Bool), m_Bool(Bool), m_Character('\0'), m_Integer(0), m_Real(0.0) {}
		Constant(char Character) :
			m_Type(BuiltInType::TypeID::m_Char),m_Bool(false), m_Character(Character), m_Integer(0), m_Real(0.0) {}
		Constant(int Integer) :
			m_Type(BuiltInType::TypeID::m_Int),m_Bool(false), m_Character('\0'), m_Integer(Integer), m_Real(0.0) {}
		Constant(double Real) :
			m_Type(BuiltInType::TypeID::m_Double), m_Bool(false), m_Character('\0'), m_Integer(0), m_Real(Real) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class Opcode: public: Expr{
	public:
		Opcode(void) {}
		virtual llvm::Value* CodeGen(CodeGenerator& Generator) = 0;
		virtual llvm::Value* CodeGenPtr(CodeGenerator& Generator) = 0;
		virtual std::string astJson() = 0;
};
class logicAND :public Opcode{   /*&&*/
  	public:
		  LogicGE(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		  llvm::Value* CodeGen(CodeGenerator& Generator);
		  llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		  std::string astJson();
    public:
		  Opcode* m_LHS;
		  Opcode* m_RHS;
  }
class LogicOR : public Opcode {
	public:
		LogicOR(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
    public:
		Opcode* LHS;
		Opcode* RHS;
	};
class BitwiseAND : public Opcode {/*&*/
	public:
		BitwiseAND(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    Opcode* m_LHS;
    Opcode* m_RHS;
	};
class BitwiseOR : public Opcode {  /*|*/
	public:
		  BitwiseOR(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		  llvm::Value* CodeGen(CodeGenerator& Generator);
		  llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		  std::string astJson();
    public:
		  Opcode* LHS;
		  Opcode* RHS;
	};
class BitwiseNot : public Opcode {  /* ~*/
	public:
		Opcode* m_Operand;
		BitwiseNot(Opcode* Operand) : m_Operand(Operand) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class BitwiseXOR : public Opcode {  /* ^ */
	public:
		BitwiseXOR(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS; 
	};
class Addition : public Opcode {   /*+*/
	public:
		Addition(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		~Addition(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class Subtraction : public Opcode {/*-*/
  	public:
		Subtraction(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		~Subtraction(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class Modulo : public Opcode {    /*%*/
	public:
		Modulo(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class Multiplication : public Opcode {  /* * */
	public:
		Multiplication(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class Division : public Opcode {   /*  /  */
	public:
		Division(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
    public:
		  Opcode* m_LHS;
		  Opcode* m_RHS;
	};
class AddAssign : public Opcode {    /*+=*/
	public:
		AddAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
    public:
  	Opcode* m_LHS;
	Opcode* m_RHS;
	};
class SubAssign : public Opcode {  /*-=*/
	public:
		SubAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		~SubAssign(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class MulAssign : public Opcode {  /* *= */
	public:
		MulAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    Opcode* m_LHS;
	Opcode* m_RHS;
	};
class DivAssign : public Opcode {   /*/=*/
	public:
		DivAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    Opcode* m_LHS;
	Opcode* m_RHS;
	};
class ModAssign : public Opcode{   /* %= */
	public:
		ModAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    	Opcode* m_LHS;
		Opcode* m_RHS;
	};
class DirectAssign : public Opcode {   /*=*/
	public:
		DirectAssign(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class LogicEQ : public Opcode {  /* == */
	public:
		LogicEQ(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    Opcode* m_LHS;
	Opcode* m_RHS;
	};
class LogicGE : public Opcode { /* >= */
  	public:
		LogicGE(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class LogicLE : public Opcode { /* <= */
	public:
		LogicLE(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class LogicNEQ : public Opcode {  /* !=*/
	public:
		LogicNEQ(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS; 
	};
class LogicLT : public Opcode { /* < */
	public:
		Opcode* LHS;
		Opcode* RHS;
		LogicLT(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    Opcode* m_LHS;
	Opcode* m_RHS;
	};
class LogicGT : public Opcode {  /* >*/
	public:
		Opcode* LHS;
		Opcode* RHS;
		LogicGT(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
    	Opcode* m_LHS;
		Opcode* m_RHS;
	};
class LogicNot : public Opcode { /* !*/
	public:
		LogicNot(Opcode* Operand) : m_Operand(Operand) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_Operand;
	};
class LeftShift : public Opcode { /* << */
	public:
		LeftShift(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class RightShift : public Opcode { /* >> */
	public:

		RightShift(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};
class TernaryCondition : public Opcode {  /* ?*/
	public:
		TernaryCondition(Opcode* Condition, Opcode* Then, Opcode* Else) : m_Condition(Condition), m_Then(Then), m_Else(Else) {}
		~TernaryCondition(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
 	public:
		Opcode* m_Condition;
		Opcode* m_Then;
		Opcode* m_Else;
	}; 
class TypeCast : public Opcode {
	public:
		VarType* m_VarType;
		Opcode* m_Operand;
		TypeCast(VarType* VarType, Opcode* Operand) : m_VarType(VarType), m_Operand(Operand) {}
		~TypeCast(void) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class StructReference : public Opcode {
	public:
		Opcode* m_Struct;
		std::string m_MemName;
		StructReference(Opcode* Struct, const std::string& MemName) : m_Struct(Struct), m_MemName(MemName) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class StructDereference : public Opcode {
	public:
		Opcode* m_StructPtr;
		std::string m_MemName;
		StructDereference(Opcode* StructPtr, const std::string& MemName) : m_StructPtr(StructPtr), m_MemName(MemName) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class Indirection : public Opcode {
	public:
		Opcode* m_Operand;
		Indirection(Opcode* Operand) : m_Operand(Operand) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class AddressOf : public Opcode {
	public:
		Opcode* m_Operand;
		AddressOf(Expr* Operand) : m_Operand(Operand) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class GlobalString : public Constant {
	public:
		std::string m_Content;
		GlobalString(const std::string& Content) : Constant(0), m_Content(Content) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class Variable : public Expr {
	public:
		std::string m_Name;
		Variable(const std::string& Name) : m_Name(Name) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
	};
class CommaExpr : public Opcode {
	public:
		CommaExpr(Opcode* LHS, Opcode* RHS) : m_LHS(LHS), m_RHS(RHS) {}
		llvm::Value* CodeGen(CodeGenerator& Generator);
		llvm::Value* CodeGenPtr(CodeGenerator& Generator);
		std::string astJson();
  	public:
		Opcode* m_LHS;
		Opcode* m_RHS;
	};







