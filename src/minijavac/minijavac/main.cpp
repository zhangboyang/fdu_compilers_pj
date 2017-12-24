#include "common.h"


int main()
{
	MiniJavaC::Instance()->OpenFile("../../../tests/Factorial.java");
	MiniJavaC::Instance()->Compile();

	system("pause");
	return 0;
}