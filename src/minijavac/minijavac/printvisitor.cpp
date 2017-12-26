#include "common.h"

void PrintVisitor::DumpASTToTextFile(const char *txtfile, ASTNode *root, bool dumpcontent)
{
	this->dumpcontent = dumpcontent;
	if (txtfile) fp = fopen(txtfile, "w"); else fp = stdout;
	root->Accept(*this);
	if (txtfile) fclose(fp);
}

void PrintVisitor::DumpTree(ASTNode *root, bool dumpcontent)
{
	fp = stdout;
	this->dumpcontent = dumpcontent;
	root->Accept(*this);
}

void PrintVisitor::Visit(ASTNode *node, int level, std::function<void()> func)
{
	for (int i = 0; i < (level + 1) * 2; i++) fprintf(fp, ">");
	fprintf(fp, " ");

	fprintf(fp, "%s [%p]: ", typeid(*node).name(), node);
	func();
	fprintf(fp, "\n");

	if (dumpcontent) MiniJavaC::Instance()->DumpContent(node->loc, fp);

	VisitChildren(node, level);
}
void PrintVisitor::Visit(ASTNode *node, int level)
{
	Visit(node, level, []{});
}
void PrintVisitor::Visit(ASTIdentifier *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "identifier=%s", node->id.c_str());
	});
}
void PrintVisitor::Visit(ASTBoolean *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "value=%d", node->val);
	});
}
void PrintVisitor::Visit(ASTNumber *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "value=%d", node->val);
	});
}
void PrintVisitor::Visit(ASTBinaryExpression *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "operator=%s", node->GetOperatorName());
	});
}
void PrintVisitor::Visit(ASTUnaryExpression *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "operator=%s", node->GetOperatorName());
	});
}
void PrintVisitor::Visit(ASTType *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "type=%s", node->GetTypeName());
	});
}