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

void MiniJavaC::DumpContent(const yyltype &loc)
{
	printf("at [(%d,%d):(%d,%d)]\n", loc.first_line, loc.first_column, loc.last_line, loc.last_column);
	int tabwidth = 4;
	char ch = ' ', ch2;
	for (int i = loc.first_line; i <= loc.last_line; i++) {
		if (i - 1 < lines.size()) {
			printf("%5u | ", i);
			for (int j = 1; j <= lines[i - 1].length(); j++) {
				if (lines[i - 1][j - 1] != '\t') {
					printf("%c", lines[i - 1][j - 1]);
				} else {
					printf("%*s", tabwidth, "");
				}
			}
			printf("%5s | ", "");
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
				printf("%c", ch2);
				if (lines[i - 1][j - 1] == '\t') {
					for (int k = 1; k < tabwidth; k++) {
						printf("%c", ch2);
					}
				}
			}
			printf("\n");
		} else {
			printf("%5s | unable to dump source code\n", "");
		}
	}
}

void MiniJavaC::ReportError(const yyltype &loc, const char *msg)
{
	DumpContent(loc);
	printf(" %s\n", msg);
}


void MiniJavaC::Compile()
{
	yycolumn = 1;
	//yydebug = 1;
	yyparse();

	ASTNodePool::Instance()->Shrink();
}
