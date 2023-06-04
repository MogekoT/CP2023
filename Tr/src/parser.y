%{
#include"AST.h"   
#include<iostream>
#include<string> 
using Decls=std::vector<Decl*>;
void yyerror(const char *s)
{
    std::printf("Error: %s\n",s);
    std::exit(1);
}
void errorout(int lineno,string s)
{
    std::printf("Error at:line: %d\n",lineno);
}
extern int yylex();
%}

%union
{
    AST::Node *node;
    AST::VarType *vartype;
    AST::Program *program;
    AST::Decl *decl;
    AST::Decls *decls;
    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Opcode *opcode;
    AST::Expr *expr;
    std::vector<Expr> *exprList;
    std::vector<VarType*> *varTypeList;
    std::vector<Opcode*> *opcodeList;
    int ival;
    double dval;
    char cval;
    std::string *sval;
    AST::AllList *defList;
    AST::AllList *ds;
    AST::CONST *const;

    /* AST::DefList *defList; */
    /* AST::VarList *varList; */
}

%token AND OR BAND BOR ADD SUB DIV MUL 
       ADDEQ SUBEQ MULEQ DIVEQ ASSIGN 
       EQ GRE SME SM GR SHL RIG QUES
       COLON SEMI COMMA LPAREN RPAREN
       LMAR RMAR LBLO RBLO BNOT POWER
       NOT NEQ PTR STRUCT UNION DEF 
       WHILE IF  RETURN MOD
       TRUE FALSE BREAK INT DOUBLE CHAR 
/* SWITCH CASE DOT*/
%token<sval> IDENTIFIER
%token<ival> INTEGER
%token<dval> REAL
%token<cval> CHARACTER
%token<sval> STRING     //UNKNOWN


%type<program>  Program
%type<decl>     Decl FuncDecl VarDecl TypeDecl 
%type<decls>    Decls 
%type<defList>  DefList
%type<vartype>  VarType VarInit BuiltInType
%type<varTypeList>  varTypeList
/* %type<defList>  DefList */
/* %type<varList>  VarList */
%type<stmt>     Stmt IfStmt WhileStmt BreakStmt ReturnStmt 
%type<stmts>    Stmts 
%type<expr>     Expr
%type<exprList> ExprList FuncUsing
%type<const> CONST;
%start program

%%
Program:
    Decls{
        $$=new AST::Program($1);
        Root=$$;
    };
Decls:
    Decls Decl{
        $$=$1;
        $$->push_back($1);
    }
    | Decl{
        $$=new AST::Decls();
    };
Decl: 
    FuncDecl 
    | VarDecl
    | TypeDecl{
        $$=$1;
    };
FuncDecl:
    VarType IDENTIFIER LPAREN DefList RPAREN SEMI{
        $$=new AST::FuncDecl($1,*$2,$4);
    }
    | VarType  IDENTIFIER LPAREN DefList RPAREN Stmts{
        $$=new AST::FuncDecl($1,*$2,$4,$6);
    };
Stmts:
    LBLO Stmts RBLO{
        $$=new AST::FuncBody($2);
    }
    |Stmts Stmt{
        $$=$1;
        if($2!=NULL) $$->push_back($2);
    }
    |Stmts{
        $$=new AST::Stmts();
    };
VarDecl: 
    VarType varTypeList SEMI{
        $$=new AST::VarDecl($1,$2);
    };
varTypeList:
    varTypeList COMMA VarInit{
        $$=$1;
        $$->push_back($3);
    }
    | VarInit{
        $$=new AST::VarTypeList($3);
        $$->push_back($1);
    }
    |{
        $$=new AST::VarTypeList();
    };
VarInit:   
    IDENTIFIER{
        $$=new AST::VarInit(*$1);
    }
    | IDENTIFIER ASSIGN Expr{
        $$=new AST::VarInit(*$1,$3);
    };
TypeDecl:
    DEF VarType IDENTIFIER SEMI{
        $$=new AST::TypeDecl($2,*$3);
    };
VarType:
    BuiltInType{
        $$=$1;
    }
    |VarType PTR{
        $$=new AST::PointerType($1);
    }
    |VarType LMAR INTEGER RMAR{
        $$=new AST::ArrayType($1,$4);
    }
    |VarType LMAR RMAR{
        $$=new AST::ArrayType($1);
    }
    | IDENTIFIER{
        $$=new AST::DefineType(*$1);
    }
    | STRUCT FuncDecl{
        return 0;       //++++
    }
    | UNION FuncDecl{
        return 0;       //++++
    };
BuiltInType:
    INT{
        $$=new AST::BuiltInType(AST::BuiltInType::TypeID::_Int);
    }
    |CHAR{
        $$=new AST::BuiltInType(AST::BuiltInType::TypeID::_Char);
    }
    |DOUBLE{
        $$=new AST::BuiltInType(AST::BuiltInType::TypeID::_Double);
    };
Stmt:
    Expr SEMI
    | IfStmt{
        $$=$1;
    }
    | WhileStmt{
        $$=$1;
    }
    | BreakStmt{
        $$=$1;
    }
    | ReturnStmt{
        $$=$1;
    }
    | SEMI{
        $$=NULL;
    };
IfStmt:
    IF LPAREN Expr RPAREN Stmt{
        $$=new AST::IfStmt($3,$5);
    }
    | IF LPAREN Expr RPAREN Stmts{
        $$=new AST::IfStmt($3,$5,$6);
    };
WhileStmt:
    WHILE LPAREN Expr RPAREN Stmt
    {
        $$=new AST::WhileStmt($3,$5);
    }
    | WHILE LPAREN Expr RPAREN Stmts{
        $$=new AST::WhileStmt($3,$5,$6);
    };
BreakStmt:
    BREAK SEMI{
        $$=new AST::BreakStmt();
    };
ReturnStmt:
    RETURN SEMI{
        $$=new AST::ReturnStmt();
    }
    | RETURN Expr SEMI{
        $$=new AST::ReturnStmt($2);
    };
//& | ! 
Expr:
    Expr POWER %prec Expr{
        $$=new AST::PowerCal($1,$3);
    }
    | IDENTIFIER LPAREN ExprList RPAREN{
        $$=new AST::FunctionCall(*$1,$3); 
    }
    | IDENTIFIER{
        $$=new AST::Variable(*$1);
    }
    | Expr DIV Expr{
        $$=new AST::Division($1,$3);
    }
    | Expr MUL Expr{
        $$=new AST::Multiplication($1,$3);
    }
    | Expr MOD Expr{
        $$=new AST::Modulo($1,$3);
    }
    | Expr ADD Expr{
        $$=new AST::Addition($1,$3);
    }
    | Expr SUB Expr{
        $$=new AST::Subtraction($1,$3);
    }
    | Expr SHL Expr{
        $$=new AST::LeftShift($1,$3);
    }
    | Expr RIG Expr{
        $$=new AST::RightShift($1,$3);
    }
    | Expr GR Expr{
        $$=new AST::LogicGR($1,$3);
    }
    | Expr GRE Expr{
        $$=new AST::LogicGRE(&1,$3);
    }
    | Expr SM Expr{
        $$=new AST::LogicSM($1,$3);
    }
    | Expr SME Expr{
        $$=new AST::LogicSME($1,$3);
    }
    | Expr NEQ Expr{
        $$=new AST::LogicNEQ($1,$3);
    }
    | Expr EQ Expr{
        $$=new AST::LogicEQ($1,$3);
    }
    | Expr OR Expr{
        $$=new AST::LogicOR($1,$3);
    }
    | Expr AND Expr{
        $$=new AST::LogicAND($1,$3);
    }
    | Expr ADDEQ Expr{
        $$=new AST::AddAssign($1,$3);
    }
    | Expr SUBEQ Expr{
        $$=new AST::SubAssign($1,$3);
    }
    | Expr MULEQ Expr{
        $$=new AST::MulAssign($1,$3);
    }
    | Expr DIVEQ Expr{
        $$=new AST::DIVAssign($1,$3);
    }
    | Expr QUES Expr COLON{
        $$=new AST::ChooseCondition($1,$3,$5);
    }
    | NOT Expr{
        $$=new AST::NotCondition($2);
    }
    | LPAREN Expr RPAREN{
        $$=$2;
    }
    | CONST{
        $$=$1;
    }
    | Expr COMMA Expr{
        $$=new AST::CommaExpr($1,$3);
    }
    | BNOT Expr{
        $$=new AST::BnotExpr($2);
    }
    | Expr BAND Expr{
        $$=new AST::BitwiseAND($1,$3);
    }
    | Expr BOR Expr{
        $$=new AST::BitwiseXOR($1,$3);
    };
ExprList:
    ExprList COMMA Expr{
        $$=$1;
        $$->push_back($3);
    }
    | Expr %prec FuncUsing{
        $$=new AST::ExprList();
        $$->push_back($1);
    }
    |{
        $$=new AST::ExprList();
    };
CONST:
    TRUE{
        $$=new AST::CONST(true);
    }
    |FALSE{
        $$=new AST::CONST(false);
    }
    |CHARACTER{
        $$=new AST::CONST($1);
    }
    |REAL{
        $$=new AST::CONST(atof($1->c_str()));
    }
    |STRING{
        $$=new AST::CONST(*$1);
    }
    |INTEGER{
        $$=new AST::CONST(atoi($1->c_str()));
    };
%%