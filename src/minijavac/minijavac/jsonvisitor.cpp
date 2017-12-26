#include "common.h"

void JSONVisitor::OutEscapedString(const char *s)
{
	while (*s) {
		switch (*s) {
			case '\"': fputs("\\\"", fp); break;
			case '\\': fputs("\\\\", fp); break;
			case '\n': fputs("\\n", fp);  break;
			case '\t': fputs("\\t", fp);  break;
			default: fputc(*s, fp); break;
		}
		s++;
	}
}
void JSONVisitor::OutQuotedString(const char *s)
{
	fputc('\"', fp);
	OutEscapedString(s);
	fputc('\"', fp);
}
void JSONVisitor::OutKeyValue(const char *key, const char *value)
{
	OutQuotedString(key);
	fputc(':', fp);
	OutQuotedString(value);
}
void JSONVisitor::OutKeyValue(const char *key, int value)
{
	OutQuotedString(key);
	fputc(':', fp);
	fprintf(fp, "%d", value);
}
void JSONVisitor::DumpASTToJSON(const char *jsonfile, ASTNode *root, const std::vector<std::string> &lines)
{
	fp = fopen(jsonfile, "w");
	assert(fp);
	
	std::string srctext;
	for (const auto &l: lines) {
		srctext += l;
	}

	fputc('{', fp);
		OutKeyValue("src", srctext.c_str()); fputc(',', fp);
		OutQuotedString("ast"); fputc(':', fp);
			fputc('[', fp);
				fputc(' ', fp); // add a space for fseek() in case there's no child
				root->Accept(*this);
				fseek(fp, -1, SEEK_CUR); // eat last comma
			fputc(']', fp);
	fputc('}', fp);

	fclose(fp);
}

void JSONVisitor::Visit(ASTNode *node, int level, std::function<void()> func)
{
	fputc('{', fp);
		OutKeyValue("type", typeid(*node).name()); fputc(',', fp);
		OutQuotedString("location"); fputc(':', fp);
			fputc('[', fp);
				fprintf(fp, "%d,", node->loc.first_line);
				fprintf(fp, "%d,", node->loc.first_column);
				fprintf(fp, "%d,", node->loc.last_line);
				fprintf(fp, "%d", node->loc.last_column);
			fputc(']', fp);
			fputc(',', fp);
		OutQuotedString("info"); fputc(':', fp);
			fputc('"', fp);
				func();
			fputc('"', fp);
			fputc(',', fp);
		OutQuotedString("children"); fputc(':', fp);
			fputc('[', fp);
				fputc(' ', fp); // add a space for fseek() in case there's no child
				VisitChildren(node, level);
				fseek(fp, -1, SEEK_CUR); // eat last comma
			fputc(']', fp);
	fputc('}', fp);
	fputc(',', fp);
}
void JSONVisitor::Visit(ASTNode *node, int level)
{
	Visit(node, level, []{});
}
void JSONVisitor::Visit(ASTIdentifier *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "identifier="); OutEscapedString(node->id.c_str());
	});
}
void JSONVisitor::Visit(ASTBoolean *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "value=%d", node->val);
	});
}
void JSONVisitor::Visit(ASTNumber *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "value=%d", node->val);
	});
}
void JSONVisitor::Visit(ASTBinaryExpression *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "operator="); OutEscapedString(node->GetOperatorName());
	});
}
void JSONVisitor::Visit(ASTUnaryExpression *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "operator="); OutEscapedString(node->GetOperatorName());
	});
}
void JSONVisitor::Visit(ASTType *node, int level)
{
	Visit(node, level, [&] {
		fprintf(fp, "type="); OutEscapedString(node->GetTypeName());
	});
}