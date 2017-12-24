%{

#include "common.h"
#include "minijavac.tab.h"

int yycolumn;

#define YY_USER_ACTION { \
		yylloc.first_line = yylineno; \
		yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng; }


#define YY_INPUT(buf,result,max_size) \
	{ \
		int c = MiniJavaC::Instance()->GetChar(); \
		if (c == '\n') { \
			yylineno++; \
			yycolumn = 0; \
		} \
		result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
	}

%}

%option noyywrap


%%

[ \n\t\r]+		{ /* ignore */ }
"class"			{ return TOK_CLASS; }

"{"				{ return TOK_LB; } // Left Braces
"}"				{ return TOK_RB; } // Reft Braces
"("				{ return TOK_LP; } // Left Parentheses
")"				{ return TOK_RP; } // Right Parentheses

"*"				{ return TOK_MUL; }
<<EOF>>			{ return TOK_EOF; }

[a-zA-Z_][a-zA-Z0-9_]*	{ return TOK_IDENTIFIER; }

.				{ return TOK_UNEXPECTED; }

%%