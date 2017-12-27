#pragma once




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
	ASTType::VarType type = ASTType::VT_UNKNOWN;
	std::string clsname;
public:
	bool operator == (const TypeInfo &r) const;
	bool operator != (const TypeInfo &r) const;
	std::string GetName();
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



////////// Instruction / Data Class //////////


class DataItem;

class RelocInfo {
public:
	enum RelocType {
		RELOC_ABS,
		RELOC_REL,
	};
	data_off_t off;
	RelocType type;
	std::shared_ptr<DataItem> target;
};

class DataBuffer;

class DataItem : public std::enable_shared_from_this<DataItem> {
	friend DataBuffer;
private:
	DataItem();
public:
	std::vector<uint8_t> bytes;
	std::vector<RelocInfo> reloc;
	std::string comment;
	data_off_t align = 1;

	// runtime data
	data_off_t off = 0;
public:
	static std::shared_ptr<DataItem> New();
	std::shared_ptr<DataItem> SetComment(const std::string &comment);
	std::shared_ptr<DataItem> AddU8(std::initializer_list<uint8_t> l);
	std::shared_ptr<DataItem> AddU32(std::initializer_list<uint32_t> l);
	std::shared_ptr<DataItem> AddRel32(uint32_t val, RelocInfo::RelocType type, std::shared_ptr<DataItem> target);
	std::shared_ptr<DataItem> AddString(const char *str);
};

class DataBuffer {
private:
	std::list<std::shared_ptr<DataItem> > list;
	std::vector<std::pair<std::string, std::shared_ptr<DataItem> > > extsym; // external reference (name, marker_to_insert)
	std::vector<std::pair<std::string, std::list<std::shared_ptr<DataItem> >::iterator > > sym; // symbols provided by current buffer (name, insert_position)
	
public:
	std::shared_ptr<DataItem> AppendItem(std::shared_ptr<DataItem> item);
	std::shared_ptr<DataItem> NewExternalSymbol(const std::string &name);
	void ProvideSymbol(const std::string &name);
	void Dump();
};



////////// CodeGen //////////


class CodeGen : public ASTNodeVisitor {
	ClassInfoList clsinfo;
public:
	DataBuffer code, rodata, data;
private:
	std::vector<TypeInfo> varstack;
	void PopAndCheckType(std::shared_ptr<ASTNode> node, TypeInfo tinfo);
	void PopAndCheckType(ASTNode *node, TypeInfo tinfo);
	void PushType(TypeInfo tinfo);

	CodeGen();
	void GenerateCodeForASTNode(std::shared_ptr<ASTNode> node);
	void GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls);
	void GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method);
public:
	virtual void Visit(ASTStatement *node, int level) override;
	virtual void Visit(ASTExpression *node, int level) override;

	// statment
	//virtual void Visit(ASTArrayAssignStatement *node, int level);
	//virtual void Visit(ASTAssignStatement *node, int level);
	virtual void Visit(ASTPrintlnStatement *node, int level);
	//virtual void Visit(ASTWhileStatement *node, int level);
	//virtual void Visit(ASTIfElseStatement *node, int level);
	//virtual void Visit(ASTBlockStatement *node, int level);

	// expression
	//virtual void Visit(ASTIdentifier *node, int level);
	//virtual void Visit(ASTBoolean *node, int level);
	virtual void Visit(ASTNumber *node, int level);
	virtual void Visit(ASTBinaryExpression *node, int level);
	//virtual void Visit(ASTUnaryExpression *node, int level);
	//virtual void Visit(ASTArrayLengthExpression *node, int level);
	//virtual void Visit(ASTFunctionCallExpression *node, int level);
	//virtual void Visit(ASTThisExpression *node, int level);
	//virtual void Visit(ASTNewIntArrayExpression *node, int level);
	//virtual void Visit(ASTNewExpression *node, int level);
public:
	static CodeGen *Instance();
	void GenerateCode();
};
