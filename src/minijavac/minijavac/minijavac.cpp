#include "common.h"
#include "minijavac.tab.h"

MiniJavaC::MiniJavaC()
{
}
MiniJavaC *MiniJavaC::Instance()
{
	static MiniJavaC inst;
	return &inst;
}
void MiniJavaC::PushFilePointer()
{
	fpstack.push_back(std::make_pair(ln, col));
}
void MiniJavaC::PopFilePointer()
{
	ln = fpstack.back().first;
	col = fpstack.back().second;
	fpstack.pop_back();
}

void MiniJavaC::OpenFile(const char *filename)
{
	lines.clear();
	lines.push_back(std::string());
	
	FILE *fp = fopen(filename, "r");
	int ch;

	while ((ch = fgetc(fp)) != EOF) {
		lines.back().push_back(ch);
		if (ch == '\n') {
			lines.push_back(std::string());
		}
	}

	fclose(fp);

	ln = col = 0;

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

void MiniJavaC::DumpContent(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2)
{
	int tabwidth = 4;
	for (unsigned i = ln1; i <= ln2; i++) {
		printf("%5u | ", ln1);
		for (unsigned j = 1; j <= lines[i - 1].length(); j++) {
			if (lines[i - 1][j - 1] != '\t') {
				printf("%c", lines[i - 1][j - 1]);
			} else {
				printf("%*s", tabwidth, "");
			}
		}
		printf("%5s | ", "");
		for (unsigned j = 1; j <= lines[i - 1].length(); j++) {
			printf("%c", (i == ln1 && j == col1) || (i == ln2 && j == col2) ? '^' : ' ');
			if (lines[i - 1][j - 1] == '\t') {
				printf("%*s", tabwidth - 1, "");
			}
		}
		printf("\n");
	}
}

void MiniJavaC::ReportError(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2, const char *msg)
{
	printf("at [(%d,%d):(%d,%d)]\n", ln1, col1, ln2, col2);
	DumpContent(ln1, col1, ln2, col2);
	printf(" %s\n", msg);
}


void MiniJavaC::Compile()
{
	yycolumn = 1;
	//yydebug = 1;
	yyparse();
}
