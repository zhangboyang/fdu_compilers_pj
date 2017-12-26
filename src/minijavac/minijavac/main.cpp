#include "common.h"


int main()
{
	//freopen("out.txt", "w", stdout);

	#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  
	#endif

	//MiniJavaC::Instance()->OpenFile("../../../tests/BinarySearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/BinaryTree.java");
	MiniJavaC::Instance()->OpenFile("../../../tests/BubbleSort.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/Factorial.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinearSearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinkedList.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/QuickSort.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/TreeVisitor.java");

	MiniJavaC::Instance()->ParseAST();
	assert(MiniJavaC::Instance()->goal);
	
	
	MiniJavaC::Instance()->DumpASTToTextFile("out.txt", true);
	
	MiniJavaC::Instance()->DumpASTToJSON("out.json");

	ClassInfoVisitor v;
	MiniJavaC::Instance()->goal->Accept(v);

	system("pause");
	return 0;
}