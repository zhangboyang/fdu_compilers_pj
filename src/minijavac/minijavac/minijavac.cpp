#include "common.h"
#include "minijavac.tab.h"

ErrFlagObj::ErrFlagObj()
{
	MiniJavaC::Instance()->errflag_stack.push_back(this);
}
ErrFlagObj::~ErrFlagObj()
{
	assert(MiniJavaC::Instance()->errflag_stack.back() == this);
	MiniJavaC::Instance()->errflag_stack.pop_back();
}

MiniJavaC::MiniJavaC()
{
}
MiniJavaC *MiniJavaC::Instance()
{
	static MiniJavaC inst;
	return &inst;
}

void MiniJavaC::LoadFile(const char *filename)
{
	assert(!src_loaded);

	printf("[*] Loading %s ...\n", filename);

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		ReportError("Can't open input file.");
		return;
	}

	int ch;

	lines.clear();
	lines.push_back(std::string());

	while ((ch = fgetc(fp)) != EOF) {
		lines.back().push_back(ch);
		if (ch == '\n') {
			lines.push_back(std::string());
		}
	}

	fclose(fp);

	ln = col = 0;

	src_loaded = true;
/*	for (auto &line : lines) {
		printf("%s", line.c_str());
	}*/
}

int MiniJavaC::GetChar()
{
	while (ln < lines.size() && col >= lines[ln].length()) {
		ln++;
		col = 0;
	}
	if (ln >= lines.size()) {
		return EOF;
	}
	return lines[ln][col++];
}

void MiniJavaC::DumpContent(const yyltype &loc)
{
	DumpContent(loc, stdout);
}
void MiniJavaC::DumpContent(const yyltype &loc, FILE *fp)
{
	fprintf(fp, " at [(%d,%d):(%d,%d)]\n", loc.first_line, loc.first_column, loc.last_line, loc.last_column);
	int tabwidth = 4;
	char ch = ' ', ch2;
	for (int i = loc.first_line; i <= loc.last_line; i++) {
		if (i - 1 < lines.size()) {
			fprintf(fp, "%5u | ", i);
			for (int j = 1; j <= lines[i - 1].length(); j++) {
				if (lines[i - 1][j - 1] != '\t') {
					fprintf(fp, "%c", lines[i - 1][j - 1]);
				} else {
					fprintf(fp, "%*s", tabwidth, "");
				}
			}
			fprintf(fp, "%5s | ", "");
			for (int j = 1; j <= lines[i - 1].length(); j++) {
				ch2 = ch;
				if (i == loc.first_line && j == loc.first_column) {
					ch = '~';
					ch2 = '^';
				}
				if (i == loc.last_line && j == loc.last_column) {
					ch = ' ';
					ch2 = '^';
				}
				fprintf(fp, "%c", ch2);
				if (lines[i - 1][j - 1] == '\t') {
					for (int k = 1; k < tabwidth; k++) {
						fprintf(fp, "%c", ch2);
					}
				}
			}
			fprintf(fp, "\n");
		} else {
			fprintf(fp, "%5s | unable to dump source code\n", "");
		}
	}
}

void MiniJavaC::ReportError(const std::string &msg, bool important)
{
	if (errflag_stack.empty() || !errflag_stack.back()->flag) {
		printf("ERROR : %s\n", msg.c_str());
		printf("\n");
		error_count++;
		if (important && !errflag_stack.empty()) errflag_stack.back()->flag = true;
	}
}
void MiniJavaC::ReportError(const yyltype &loc, const std::string &msg, bool important)
{
	if (errflag_stack.empty() || !errflag_stack.back()->flag) {
		DumpContent(loc);
		ReportError(msg);
		if (important && !errflag_stack.empty()) errflag_stack.back()->flag = true;
	}
}


void MiniJavaC::ParseAST()
{
	printf("[*] Generating AST ...\n");
	yycolumn = 1;
	//yydebug = 1;
	yyparse();
	ASTNodePool::Instance()->Shrink();
}

void MiniJavaC::DumpASTToTextFile(const char *txtfile, bool dumpcontent)
{
	PrintVisitor v;
	v.DumpASTToTextFile(txtfile, goal.get(), dumpcontent);
}

void MiniJavaC::DumpASTToJSON(const char *jsonfile)
{
	JSONVisitor v;
	v.DumpASTToJSON(jsonfile, goal.get(), lines);
}