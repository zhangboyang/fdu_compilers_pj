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

template<class T>
class NameIndexedList : public std::vector<T> {
private:
	std::map<std::string, size_t> listindex;
public:
	bool Append(const T &item)
	{
		if (listindex.insert(std::make_pair(item.GetName(), std::vector<T>::size())).second) {
			std::vector<T>::push_back(item);
			return true;
		} else {
			return false;
		}
	}
	std::vector<T>::iterator Find(const std::string &name)
	{
		auto it = listindex.find(name);
		if (it == listindex.end()) {
			return end();
		} else {
			return begin() + it->second;
		}
	}
};

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

class VarDeclItem {
public:
	VarDecl decl;
	data_off_t off;
	data_off_t size;
public:
	const std::string &GetName() const;
};

class VarDeclList : public NameIndexedList<VarDeclItem> {
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

class MethodDeclItem {
public:
	MethodDecl decl;
	data_off_t off;
	VarDeclList localvar;
	std::shared_ptr<ASTMethodDeclaration> ptr;
public:
	const std::string &GetName() const;
};

class MethodDeclList : public NameIndexedList<MethodDeclItem> {
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
	const std::string &GetName() const;
	void Dump();
};

class ClassInfoList : public NameIndexedList<ClassInfoItem> {
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





class CodeGen : public ASTNodeVisitor {
	ClassInfoList clsinfo;

private:
	CodeGen();
	void GenerateCodeForASTNode(std::shared_ptr<ASTNode> node);
	void GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls);
	void GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method);
public:
	virtual void Visit(ASTStatement *node, int level) override;
	virtual void Visit(ASTExpression *node, int level) override;
public:
	static CodeGen *Instance();

	void GenerateCode();
};
