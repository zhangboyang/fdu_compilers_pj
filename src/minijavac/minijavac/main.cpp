#include "common.h"


int main()
{
	printf("Hello World!\n");

	MiniJavaC::Instance()->OpenFile("../../../tests/Factorial.java");
	MiniJavaC::Instance()->Compile();

	system("pause");
	return 0;
}