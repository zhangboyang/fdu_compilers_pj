#pragma once

#define YYLTYPE yyltype
struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};

#define YYSTYPE ASTNode *

extern int yycolumn;
extern int yylex();
extern void yyerror(const char *s);

#include <vector>

class MiniJavaC {
	std::vector<std::string> lines;
	unsigned ln, col;

private:
	MiniJavaC();
public:
	int GetChar();
	void ReportError(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2, const char *msg);
	static MiniJavaC *Instance();

	void OpenFile(const char *filename);
	void DumpContent(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2);
	void Compile();
};


