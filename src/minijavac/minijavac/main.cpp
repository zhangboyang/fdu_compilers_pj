#include "common.h"


int main(int argc, char *argv[])
{
	//freopen("out.txt", "w", stdout);

	#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  
	#endif

	#ifdef _DEBUG
	//MiniJavaC::Instance()->LoadFile("test.java");


	//MiniJavaC::Instance()->LoadFile("../../../mytests/duplicate.java");

	//MiniJavaC::Instance()->LoadFile("../../../tests/BinarySearch.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/BinaryTree.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/BubbleSort.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/Factorial.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/LinearSearch.java");
	MiniJavaC::Instance()->LoadFile("../../../tests/LinkedList.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/QuickSort.java");
	//MiniJavaC::Instance()->LoadFile("../../../tests/TreeVisitor.java");
	#else

	if (argc >= 2) {
		MiniJavaC::Instance()->LoadFile(argv[1]);
	}
	
	#endif



	if (MiniJavaC::Instance()->src_loaded) {
		MiniJavaC::Instance()->ParseAST();
		if (MiniJavaC::Instance()->goal) {
			MiniJavaC::Instance()->DumpASTToTextFile("out.ast.txt", true);
			MiniJavaC::Instance()->DumpASTToJSON("out.ast.json"); 
			CodeGen::Instance()->GenerateCode();
			CodeGen::Instance()->DumpSections("out.asm.txt");
		}
	}

	int err_cnt = MiniJavaC::Instance()->error_count;
	if (err_cnt) {
		printf("\n%d error(s) occured, compile failed.\n\n", err_cnt);
	} else {
		printf("\nCompile successful.\n\n");
	}

	#ifdef _DEBUG
	system("pause");
	#endif

	return !!err_cnt;
}