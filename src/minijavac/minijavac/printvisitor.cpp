#include "common.h"

void PrintVisitor::Visit(ASTNode *node, int level, std::function<void()> func)
{
	for (int i = 0; i < (level + 1) * 2; i++) printf(">");
	printf(" ");

	printf("%s [%p]: ", typeid(*node).name(), node);
	func();
	printf("\n");

	MiniJavaC::Instance()->DumpContent(node->loc);

	VisitChildren(node, level);
}
void PrintVisitor::Visit(ASTNode *node, int level)
{
	Visit(node, level, []{});
}
void PrintVisitor::Visit(ASTIdentifier *node, int level)
{
	Visit(node, level, [&] {
		printf("identifier=%s", node->id.c_str());
	});
}
void PrintVisitor::Visit(ASTBoolean *node, int level)
{
	Visit(node, level, [&] {
		printf("value=%d", node->val);
	});
}
void PrintVisitor::Visit(ASTNumber *node, int level)
{
	Visit(node, level, [&] {
		printf("value=%d", node->val);
	});
}
void PrintVisitor::Visit(ASTBinaryExpression *node, int level)
{
	Visit(node, level, [&] {
		printf("operator=%s", node->GetOperatorName());
	});
}
void PrintVisitor::Visit(ASTUnaryExpression *node, int level)
{
	Visit(node, level, [&] {
		printf("operator=%s", node->GetOperatorName());
	});
}
void PrintVisitor::Visit(ASTType *node, int level)
{
	Visit(node, level, [&] {
		printf("type=%s", node->GetTypeName());
	});
}