%{

#include "minijavac.h"

%}

%define parse.error verbose
%define parse.trace true


%token TOK_EOF
%token TOK_MUL

%%
Goal
  : TOK_EOF

%%
