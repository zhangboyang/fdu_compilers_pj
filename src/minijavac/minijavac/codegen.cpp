#include "common.h"

void ClassInfoVisitor::Visit(ASTClassDeclaration *node, int level)
{
	node->Dump();
}