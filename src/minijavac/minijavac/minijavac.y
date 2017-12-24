%{
#include "common.h"
%}

%locations


%define parse.error verbose
%define parse.trace true


%token TOK_EOF
%token TOK_CLASS
%token TOK_MUL
%token TOK_IDENTIFIER

%%
Goal
  : TOK_CLASS TOK_EOF

%%

void yyerror(const char *s)
{
	MiniJavaC::Instance()->ReportError(yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, s);
}

