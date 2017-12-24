#include "common.h"

#include "minijavac.h"

void yyerror(const char *s)
{
	fprintf(stderr, "%s\n", s);
}
