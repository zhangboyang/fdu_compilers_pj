#include "common.h"

void PrintVisitor::Visit(ASTNode *node, int level)
{
	level = (level + 1) * 2;
	while (level--) printf(">");
	printf(" ");

	printf("%s [%p]\n", typeid(*node).name(), node);
	MiniJavaC::Instance()->DumpContent(node->loc);
}