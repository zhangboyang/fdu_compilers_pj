%{

#include "common.h"
#include "minijavac.tab.h"

int yycolumn = 1;

#define YY_USER_ACTION { \
		yylloc.first_line = yylineno; \
		yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng; }


#define YY_INPUT(buf,result,max_size) \
	{ \
		int c = MiniJavaC::Instance()->GetChar(); \
		result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
	}

%}

%option noyywrap


%%

[ \t\r]+		{ /* ignore */ }
"class"         { return TOK_CLASS; }
"*"				{ return TOK_MUL; }
<<EOF>>			{ return TOK_EOF; }

[a-zA-Z_][a-zA-Z0-9_]*	{ return TOK_IDENTIFIER; }

%%