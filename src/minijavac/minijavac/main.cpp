#include "common.h"


int main()
{
	//MiniJavaC::Instance()->OpenFile("../../../tests/BinarySearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/BinaryTree.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/BubbleSort.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/Factorial.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinearSearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinkedList.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/QuickSort.java");
	MiniJavaC::Instance()->OpenFile("../../../tests/TreeVisitor.java");
	MiniJavaC::Instance()->Compile();

	system("pause");
	return 0;
}