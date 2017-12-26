#pragma once

////////// Instruction / Data Class //////////
typedef int data_off_t;

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

class VarDecl {
public:
	TypeInfo type;
	std::string name;
};

class MethodDecl {
public:
	TypeInfo rettype;
	std::vector<VarDecl> arg;
};

class VarListItem {
public:
	VarDecl decl;
	data_off_t off;
	data_off_t size;
};

class MethodListItem {
public:
	MethodDecl decl;
	data_off_t off;
};

class ClassInfo {
public:
	std::string name;
	std::vector<VarListItem> var;
	std::vector<MethodListItem> method;
};


// Visitor

class ClassInfoVisitor : public ASTNodeVisitor {
public:
	virtual void Visit(ASTClassDeclaration *node, int level) override;
};


class MethodInfo {
	
};

class MethodInfoVisitor {

};

class CodeGenVisitor : public ASTNodeVisitor {
	
};
