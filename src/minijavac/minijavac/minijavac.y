%{
#include "common.h"


%}

%locations
%debug
%glr-parser

%define parse.error verbose


%token TOK_CLASS
%token TOK_PUBLIC
%token TOK_STATIC
%token TOK_VOID
%token TOK_MAIN
%token TOK_INT
%token TOK_STRING
%token TOK_BOOLEAN
%token TOK_ELSE
%token TOK_IF
%token TOK_FALSE
%token TOK_TRUE
%token TOK_LAND
%token TOK_LENGTH
%token TOK_LT
%token TOK_NUM
%token TOK_PRINTLN
%token TOK_RETURN
%token TOK_WHILE

%token TOK_THIS
%token TOK_NEW

%token TOK_LP TOK_RP
%token TOK_LB TOK_RB
%token TOK_LS TOK_RS

%token TOK_EQUAL
%token TOK_MUL
%token TOK_ADD
%token TOK_SUB
%token TOK_NOT
%token TOK_DOT
%token TOK_COM
%token TOK_SEMI
%token TOK_IDENTIFIER
%token TOK_UNEXPECTED


%left TOK_EQUAL
%left TOK_LAND
%left TOK_LT
%left TOK_ADD TOK_SUB
%left TOK_MUL
%right TOK_NOT
%left TOK_DOT


%%
Goal
  : MainClass ClassDeclarationList
    { MiniJavaC::Instance()->goal = (new ASTGoal(@$, { $1, $2 }))->GetSharedPtr(); }
;

ClassDeclarationList
  :
    { $$ = new ASTClassDeclarationList(@$); }
  | ClassDeclarationList ClassDeclaration
    { $$ = new ASTClassDeclarationList(@$, { $1, $2 }); }
;

MainClass
  : TOK_CLASS Identifier TOK_LB TOK_PUBLIC TOK_STATIC TOK_VOID TOK_MAIN TOK_LP TOK_STRING TOK_LS TOK_RS Identifier TOK_RP TOK_LB Statement TOK_RB TOK_RB
    { $$ = new ASTMainClass(@$, { $2, $12, $15 }); }
;

ClassDeclaration
  : TOK_CLASS Identifier TOK_LB VarDeclarationList MethodDeclarationList TOK_RB
    { $$ = new ASTClassDeclaration(@$, { $2, $4, $5 }); }
;

VarDeclarationList
  :
	{ $$ = new ASTVarDeclarationList(@$); }
  | VarDeclarationList VarDeclaration
    { $$ = new ASTVarDeclarationList(@$, { $1, $2 }); }
;

MethodDeclarationList
  :
	{ $$ = new ASTMethodDeclarationList(@$); }
  | MethodDeclarationList MethodDeclaration
    { $$ = new ASTMethodDeclarationList(@$, { $1, $2 }); }
;

VarDeclaration
  : Type Identifier TOK_SEMI
    { $$ = new ASTVarDeclaration(@$, { $1, $2 }); }
;

MethodDeclaration
  : TOK_PUBLIC Type Identifier TOK_LP ArgDeclarationList1 TOK_RP TOK_LB VarDeclarationList StatementList TOK_RETURN Expression TOK_SEMI TOK_RB
    { $$ = new ASTMethodDeclaration(@$, { $2, $3, $5, $8, $9, $11 }); }
;

ArgDeclarationList1
  :
	{ $$ = new ArgDeclarationList1(@$); }
  | Type Identifier ArgDeclarationList2
    { $$ = new ArgDeclarationList1(@$, { $1, $2, $3 }); }
;

ArgDeclarationList2
  :
	{ $$ = new ArgDeclarationList2(@$); }
  | ArgDeclarationList2 TOK_COM Type Identifier
	{ $$ = new ArgDeclarationList2(@$, { $1, $3, $4 }); }
;

Type
  : TOK_INT TOK_LS TOK_RS
    { $$ = new ASTType(@$, {}, ASTType::VT_INTARRAY); }
  | TOK_BOOLEAN
    { $$ = new ASTType(@$, {}, ASTType::VT_BOOLEAN); }
  | TOK_INT
    { $$ = new ASTType(@$, {}, ASTType::VT_INT); }
  | Identifier
    { $$ = new ASTType(@$, { $1 }, ASTType::VT_CLASS); }
;

Statement
  : TOK_LB StatementList TOK_RB
    { $$ = new ASTBlockStatement(@$, { $2 }); }
  | TOK_IF TOK_LP Expression TOK_RP Statement TOK_ELSE Statement
    { $$ = new ASTIfElseStatement(@$, { $3, $5, $7 }); }
  | TOK_WHILE TOK_LP Expression TOK_RP Statement
	{ $$ = new ASTWhileStatement(@$, { $3, $5 }); }
  | TOK_PRINTLN TOK_LP Expression TOK_RP TOK_SEMI
    { $$ = new ASTPrintlnStatement(@$, { $3 }); }
  | Identifier TOK_EQUAL Expression TOK_SEMI
    { $$ = new ASTAssignStatement(@$, { $1, $3 }); }
  | Identifier TOK_LS Expression TOK_RS TOK_EQUAL Expression TOK_SEMI
    { $$ = new ASTArrayAssignStatement(@$, { $1, $3, $6 }); }
;

StatementList
  :
	{ $$ = new ASTStatementList(@$); }
  | StatementList Statement
    { $$ = new ASTStatementList(@$, { $1, $2} ); }
;

Expression
  : Expression TOK_LAND Expression
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_LAND); }
  | Expression TOK_LT Expression
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_LT); }
  | Expression TOK_ADD Expression
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_ADD); }
  | Expression TOK_SUB Expression
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_SUB); }
  | Expression TOK_MUL Expression
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_MUL); }
  | Expression TOK_LS Expression TOK_RS
    { $$ = new ASTBinaryExpression(@$, { $1, $3 }, TOK_LS); }
  | Expression TOK_DOT TOK_LENGTH
	{ $$ = new ASTArrayLengthExpression(@$, { $1 }); }
  | Expression TOK_DOT Identifier TOK_LP ArgExpressionList1 TOK_RP
    { $$ = new ASTFunctionCallExpression(@$, { $1, $3, $5 }); }
  | TOK_NUM
    { $$ = $1; }
  | TOK_TRUE
    { $$ = $1; }
  | TOK_FALSE
    { $$ = $1; }
  | Identifier
    { $$ = $1; }
  | TOK_THIS
    { $$ = new ASTThisExpression(@$); }
  | TOK_NEW TOK_INT TOK_LS Expression TOK_RS
    { $$ = new ASTNewIntArrayExpression(@$, { $4 }); }
  | TOK_NEW Identifier TOK_LP TOK_RP
	{ $$ = new ASTNewExpression(@$, { $2 }); }
  | TOK_NOT Expression
	{ $$ = new ASTUnaryExpression(@$, { $2 }, TOK_NOT); }
  | TOK_LP Expression TOK_RP
    { $$ = new ASTUnaryExpression(@$, { $2 }, TOK_LP); }
;

ArgExpressionList1
  :
	{ $$ = new ASTArgExpressionList1(@$); }
  | Expression ArgExpressionList2
	{ $$ = new ASTArgExpressionList1(@$, { $1, $2 }); }
;

ArgExpressionList2
  :
	{ $$ = new ASTArgExpressionList2(@$); }
  | ArgExpressionList2 TOK_COM Expression
	{ $$ = new ASTArgExpressionList2(@$, { $1, $3 }); }
;

Identifier
  : TOK_IDENTIFIER
    { $$ = $1; }
;
%%

void yyerror(const char *s)
{
	MiniJavaC::Instance()->ReportError(yylloc, s);
}

