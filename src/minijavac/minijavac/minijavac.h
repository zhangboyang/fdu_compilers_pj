#pragma once

extern int yylex();
extern void yyerror(const char *s);

#include <vector>

class MiniJavaC {
	std::vector<std::string> lines;
	std::vector<std::pair<unsigned, unsigned> > fpstack;
	unsigned ln, col;

private:
	MiniJavaC();
	void PushFilePointer();
	void PopFilePointer();
public:
	int GetChar();
	void ReportError(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2, const char *msg);
	static MiniJavaC *Instance();

	void OpenFile(const char *filename);
	void DumpContent(unsigned ln1, unsigned col1, unsigned ln2, unsigned col2);
	void Compile();
};