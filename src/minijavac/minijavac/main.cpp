#include "common.h"
#include "minijavac.tab.h"

extern FILE *yyin;

int main()
{
	printf("Hello World!\n");

	yyin = fopen("../../../tests/Factorial.java", "r");
	yyparse();

	system("pause");
	return 0;
}