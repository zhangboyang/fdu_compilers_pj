#pragma once

////////// Instruction / Data Class //////////


class DataItem;

class RelocInfo {
public:
	enum RelocType {
		RELOC_ABS,
		RELOC_REL,
	};
	RelocType type;
	std::shared_ptr<DataItem> target;
};

class DataItem {
public:
	std::vector<uint8_t> bytes;
	data_off_t off = 0;
	std::vector<RelocInfo> reloc;
};

class Instr: public DataItem { // instruction
	using DataItem::DataItem;
};

class MarkerItem : public DataItem { // position marker
	using DataItem::DataItem;
};

class X86Instr : public Instr {
	using Instr::Instr;
};

class X86LinkInstr : public X86Instr {
};

class DataBuffer {
private:
	std::list<std::shared_ptr<DataItem> > list;
public:
	void AppendItem(std::shared_ptr<DataItem> instr);
	std::shared_ptr<MarkerItem> AppendMarker();
};




////////// CodeGen Visitor //////////

class TypeInfo {
public:
	ASTType::VarType type;
	std::string clsname;
};


// VarDecl

class VarDecl {
public:
	TypeInfo type;
	std::string name;
};

class VarDeclListItem {
public:
	VarDecl decl;
	data_off_t off;
	data_off_t size;
};

class VarDeclList : public std::vector<VarDeclListItem> {
public:
	void Dump();
	data_off_t GetTotalSize();
};

// MethodDecl

class MethodDecl {
public:
	TypeInfo rettype;
	std::string name;
	VarDeclList arg;
};

class MethodDeclListItem {
public:
	MethodDecl decl;
	data_off_t off;
	std::shared_ptr<ASTMethodDeclaration> ptr;
};

class MethodDeclList : public std::vector<MethodDeclListItem> {
public:
	void Dump();
	data_off_t GetTotalSize();
};




class ClassInfoItem {
public:
	std::string name;
	VarDeclList var;
	MethodDeclList method;
	std::shared_ptr<ASTClassDeclaration> ptr;
public:
	void Dump();
};

class ClassInfoList : public std::vector<ClassInfoItem> {
public:
	void Dump();
};

// Visitor

class VarDeclListVisitor : public ASTNodeVisitor {
public:
	VarDeclList list;
	virtual void Visit(ASTVarDeclaration *node, int level) override;
};

class MethodDeclListVisitor : public ASTNodeVisitor {
public:
	MethodDeclList list;
	virtual void Visit(ASTMethodDeclaration *node, int level) override;
};

class ClassInfoVisitor : public ASTNodeVisitor {
public:
	ClassInfoList list;
	virtual void Visit(ASTClassDeclaration *node, int level) override;
};


class MethodInfo {
	
};

class MethodInfoVisitor {

};

class CodeGenVisitor : public ASTNodeVisitor {
	
};
