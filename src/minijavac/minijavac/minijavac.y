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
;

ClassDeclarationList
  :
  | ClassDeclarationList ClassDeclaration
;

MainClass
  : TOK_CLASS Identifier TOK_LB TOK_PUBLIC TOK_STATIC TOK_VOID TOK_MAIN TOK_LP TOK_STRING TOK_LS TOK_RS Identifier TOK_RP TOK_LB Statement TOK_RB TOK_RB
;

ClassDeclaration
  : TOK_CLASS Identifier TOK_LB VarDeclarationList MethodDeclarationList TOK_RB
;

VarDeclarationList
  :
  | VarDeclarationList VarDeclaration
;

MethodDeclarationList
  :
  | MethodDeclarationList MethodDeclaration
;

VarDeclaration
  : Type Identifier TOK_SEMI
;

MethodDeclaration
  : TOK_PUBLIC Type Identifier TOK_LP ArgDeclarationList1 TOK_RP TOK_LB VarDeclarationList StatementList TOK_RETURN Expression TOK_SEMI TOK_RB
;

ArgDeclarationList1
  :
  | Type Identifier ArgDeclarationList2
;

ArgDeclarationList2
  :
  | ArgDeclarationList2 TOK_COM Type Identifier
;

Type
  : TOK_INT TOK_LS TOK_RS
  | TOK_BOOLEAN
  | TOK_INT
  | Identifier
;

Statement
  : TOK_LB StatementList TOK_RB
  | TOK_IF TOK_LP Expression TOK_RP Statement TOK_ELSE Statement
  | TOK_WHILE TOK_LP Expression TOK_RP Statement
  | TOK_PRINTLN TOK_LP Expression TOK_RP TOK_SEMI
  | Identifier TOK_EQUAL Expression TOK_SEMI
  | Identifier TOK_LS Expression TOK_RS TOK_EQUAL Expression TOK_SEMI
;

StatementList
  :
  | StatementList Statement
;	

Expression
  : Expression TOK_LAND Expression
  | Expression TOK_LT Expression
  | Expression TOK_ADD Expression
  | Expression TOK_SUB Expression
  | Expression TOK_MUL Expression
  | Expression TOK_LS Expression TOK_RS
  | Expression TOK_DOT TOK_LENGTH
  | Expression TOK_DOT Identifier TOK_LP ExpressionList1 TOK_RP
  | TOK_NUM
  | TOK_TRUE
  | TOK_FALSE
  | Identifier
  | TOK_THIS
  | TOK_NEW TOK_INT TOK_LS Expression TOK_RS
  | TOK_NEW Identifier TOK_LP TOK_RP
  | TOK_NOT Expression
  | TOK_LP Expression TOK_RP
;

ExpressionList1
  :
  | Expression ExpressionList2
;

ExpressionList2
  :
  | ExpressionList2 TOK_COM Expression
;

Identifier
  : TOK_IDENTIFIER
;
%%

void yyerror(const char *s)
{
	MiniJavaC::Instance()->ReportError(yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, s);
}

