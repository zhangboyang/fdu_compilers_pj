#include "common.h"

void PrintVisitor::Visit(ASTNode *node, int level)
{
	printf("%*s%s [%p]\n", level * 2, "", typeid(*node).name(), node);
}