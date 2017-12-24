%{
#include "common.h"
%}

%locations


%define parse.error verbose
%define parse.trace true


%token TOK_EOF
%token TOK_CLASS
%token TOK_LP TOK_RP
%token TOK_LB TOK_RB
%token TOK_MUL
%token TOK_IDENTIFIER
%token TOK_UNEXPECTED

%%
Goal
  : MainClass TOK_EOF
;

MainClass
  : TOK_CLASS TOK_IDENTIFIER TOK_LB TOK_IDENTIFIER
;

%%

void yyerror(const char *s)
{
	MiniJavaC::Instance()->ReportError(yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, s);
}

