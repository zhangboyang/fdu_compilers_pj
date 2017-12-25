#include "common.h"


int main()
{
	#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  
	#endif

	//MiniJavaC::Instance()->OpenFile("../../../tests/BinarySearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/BinaryTree.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/BubbleSort.java");
	MiniJavaC::Instance()->OpenFile("../../../tests/Factorial.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinearSearch.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/LinkedList.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/QuickSort.java");
	//MiniJavaC::Instance()->OpenFile("../../../tests/TreeVisitor.java");
	MiniJavaC::Instance()->Compile();


	std::shared_ptr<ASTNode> p = (new ASTIdentifier())->GetSharedPtr();

	p->AddChild(new ASTNode());

	new ASTNode { new ASTNode(), new ASTNode(), new ASTNode() };

	printf("%s\n", typeid(*p).name());

	ASTNodePool::Instance()->Shrink();

	system("pause");
	return 0;
}