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
			return std::vector<T>::end();
		} else {
			return std::vector<T>::begin() + it->second;
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
	bool CanCastTo(const TypeInfo &r) const;
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
	void Dump(FILE *fp);
	data_off_t GetTotalSize();
};

// MethodDecl

class MethodDecl {
public:
	TypeInfo rettype;
	std::string name;
	VarDeclList arg;
	bool operator == (const MethodDecl &r) const;
};

class MethodDeclItem {
public:
	MethodDecl decl;
	data_off_t off;
	VarDeclList localvar;
	std::string clsname;
	std::shared_ptr<ASTMethodDeclaration> ptr;
public:
	const std::string &GetName() const;
};

class MethodDeclList : public NameIndexedList<MethodDeclItem> {
public:
	void Dump(FILE *fp);
	data_off_t GetTotalSize();
};




class ClassInfoItem {
public:
	std::string name;
	VarDeclList var;
	MethodDeclList method;
	std::string base;
public:
	const std::string &GetName() const;
	void Dump(FILE *fp);
};

class ClassInfoList : public NameIndexedList<ClassInfoItem> {
public:
	void Dump(FILE *fp);
};

// Visitor

class VarDeclListVisitor : public ASTNodeVisitor {
public:
	VarDeclListVisitor(VarDeclList base);
	VarDeclList list;
	virtual void Visit(ASTVarDeclaration *node, int level) override;
};

class MethodDeclListVisitor : public ASTNodeVisitor {
	std::string clsname;
public:
	MethodDeclListVisitor(MethodDeclList base, const std::string &clsname);
	MethodDeclList list;
	virtual void Visit(ASTMethodDeclaration *node, int level) override;
};

class ClassInfoVisitor : public ASTNodeVisitor {
public:
	ClassInfoList list;
	virtual void Visit(ASTClassDeclaration *node, int level) override;
	virtual void Visit(ASTDerivedClassDeclaration *node, int level) override;
};



////////// Instruction / Data Class //////////


class DataItem;

class RelocInfo {
public:
	enum RelocType {
		RELOC_ABS32,
		RELOC_REL32,
		RELOC_RVA32,
	};
	data_off_t off;
	RelocType type;
	std::shared_ptr<DataItem> target;
	bool done = false;
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
	uint8_t align_fill = 0;

	// runtime data
	data_off_t off = 0;
public:
	static std::shared_ptr<DataItem> New();
	std::shared_ptr<DataItem> SetComment(const std::string &comment);
	std::shared_ptr<DataItem> AddU8(std::initializer_list<uint8_t> l);
	std::shared_ptr<DataItem> AddU32(std::initializer_list<uint32_t> l);
	std::shared_ptr<DataItem> AddRel32(uint32_t val, RelocInfo::RelocType type, std::shared_ptr<DataItem> target);
	std::shared_ptr<DataItem> AddString(const char *str);
	std::shared_ptr<DataItem> SetAlign(data_off_t align, uint8_t fill = 0);
};

class DataBuffer {
private:
	std::list<std::shared_ptr<DataItem> > list;
	std::vector<std::pair<std::string, std::pair<std::shared_ptr<DataItem>, bool> > > extsym; // external reference (name, (marker_to_insert, done_flag))
	std::vector<std::pair<std::string, std::pair<std::list<std::shared_ptr<DataItem> > *, std::list<std::shared_ptr<DataItem> >::iterator > > > sym; // symbols provided by current buffer (name, (list_ptr, insert_position))
public:
	data_off_t base_addr;
	data_off_t end_addr;
public:
	std::shared_ptr<DataItem> AppendItem(std::shared_ptr<DataItem> item);
	std::shared_ptr<DataItem> NewExternalSymbol(const std::string &name);
	data_off_t CalcOffset(data_off_t base);
	void DoRelocate();
	void ProvideSymbol(const std::string &name);
	void Dump(FILE *fp);
	static void ReduceSymbols(const std::vector<DataBuffer *> buffers);
	data_off_t GetSymbol(const std::string &symname);
	std::vector<uint8_t> GetContent();
};



////////// CodeGen //////////


class CodeGen : public ASTNodeVisitor {
	static const unsigned PE_TOTAL_SECTIONS = 3;
	static const unsigned PE_IMAGEBASE = 0x00400000;
	static const unsigned PE_SECTIONALIGN = 0x1000;
	static const unsigned PE_CODEBASE = 0x1000;
	static const unsigned PE_FILEALIGN = 0x1000;
private:
	std::vector<TypeInfo> varstack;
	ClassInfoItem *cur_cls;
	MethodDeclItem *cur_method;
	DataBuffer code, rodata, data;
public:
	ClassInfoList clsinfo;
private:
	void AssertTypeEmpty(const yyltype &loc);
	TypeInfo PopType();
	void PopAndCheckType(const yyltype &loc, TypeInfo tinfo);
	void PushType(TypeInfo tinfo);

	void LoadThisToEAX();

	// get local-var info (arg and stack var)
	// return < <bp-offset, size>, type>
	// return < <0,0>, VT_UNKNOWN > if not found
	std::pair<std::pair<data_off_t, data_off_t>, TypeInfo> GetLocalVar(const std::string &name);

	// get member-var info
	// return < <this-offset, size>, type>
	// return < <0,0>, VT_UNKNOWN > if not found
	std::pair<std::pair<data_off_t, data_off_t>, TypeInfo> GetMemberVar(const std::string &name);

	// dllinfo
	std::vector<std::pair<std::string, std::vector<std::string> > > dllinfo; // <dllname, funclist>

	CodeGen();
	void GenerateCodeForASTNode(std::shared_ptr<ASTNode> node);
	void GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls);
	void GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method);

	void GenerateVtblForClass(ClassInfoItem &cls);

	void MakeIAT();
	void AddImportEntry(const std::string &dllname, const std::vector<std::string> &funclist);
	void MakeEXE();
	
	data_off_t GetSymbol(const std::string &sym);
	

	void Link();

public:
	virtual void Visit(ASTStatement *node, int level) override;
	virtual void Visit(ASTExpression *node, int level) override;

	// statment
	virtual void Visit(ASTArrayAssignStatement *node, int level);
	virtual void Visit(ASTAssignStatement *node, int level);
	virtual void Visit(ASTPrintlnStatement *node, int level);
	virtual void Visit(ASTWhileStatement *node, int level);
	virtual void Visit(ASTIfElseStatement *node, int level);
	virtual void Visit(ASTBlockStatement *node, int level);

	// expression
	virtual void Visit(ASTIdentifier *node, int level);
	virtual void Visit(ASTBoolean *node, int level);
	virtual void Visit(ASTNumber *node, int level);
	virtual void Visit(ASTBinaryExpression *node, int level);
	virtual void Visit(ASTUnaryExpression *node, int level);
	virtual void Visit(ASTArrayLengthExpression *node, int level);
	virtual void Visit(ASTFunctionCallExpression *node, int level);
	virtual void Visit(ASTThisExpression *node, int level);
	virtual void Visit(ASTNewIntArrayExpression *node, int level);
	virtual void Visit(ASTNewExpression *node, int level);
public:
	static CodeGen *Instance();
	void GenerateCode();
	void DumpSections(const char *outfile);
	void DumpVars(const char *outfile);
	static data_off_t ToRVA(data_off_t addr);
};
