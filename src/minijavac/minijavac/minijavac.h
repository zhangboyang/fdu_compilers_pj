#pragma once

#include <cstdio>

extern FILE * yyin;
extern int yylex();
void yyerror(const char *s);