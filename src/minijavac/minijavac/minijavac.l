%{

#include "minijavac.h"
#include "minijavac.tab.h"

%}

%option noyywrap


%%

<<EOF>>			{ return TOK_EOF; }
[ \t\r]+		{ /* ignore */ }
"*"				{ return TOK_MUL; }

%%