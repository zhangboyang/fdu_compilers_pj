#pragma once

#define YYLTYPE yyltype
struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};

class ASTNode;

#define YYSTYPE ASTNode *

extern int yycolumn;
extern int yylex();
extern void yyerror(const char *s);


////// the MiniJavaC class //////

class ASTGoal;

class MiniJavaC {
	std::vector<std::string> lines;
	unsigned ln, col;

public:
	std::shared_ptr<ASTGoal> goal;

private:
	MiniJavaC();
public:
	int GetChar();
	void ReportError(const yyltype &loc, const std::string &msg);
	void ReportError(const std::string &msg);
	static MiniJavaC *Instance();

	void OpenFile(const char *filename);
	void DumpContent(const yyltype &loc, FILE *fp);
	void DumpContent(const yyltype &loc);
	void ParseAST();
	void DumpASTToTextFile(const char *txtfile, bool dumpcontent);
	void DumpASTToJSON(const char *jsonfile);
};


