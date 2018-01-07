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

class ErrFlagObj {
public:
	bool flag = false;
	ErrFlagObj();
	~ErrFlagObj();
};

class MiniJavaC {
	friend class ErrFlagObj;

	std::vector<std::string> lines;
	unsigned ln, col;
	std::vector<ErrFlagObj *> errflag_stack;

public:
	std::shared_ptr<ASTGoal> goal;
	bool src_loaded = false;
	int error_count = 0;

private:
	MiniJavaC();
public:
	int GetChar();
	void ReportError(const yyltype &loc, const std::string &msg, bool important = false);
	void ReportError(const std::string &msg, bool important = false);
	static MiniJavaC *Instance();

	void LoadFile(const char *filename);
	void DumpContent(const yyltype &loc, FILE *fp);
	void DumpContent(const yyltype &loc);
	void ParseAST();
	void DumpASTToTextFile(const char *txtfile, bool dumpcontent);
	void DumpASTToJSON(const char *jsonfile);
};


