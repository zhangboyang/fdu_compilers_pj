#include "common.h"
#include "minijavac.tab.h"

const std::string &VarDeclItem::GetName() const
{
	return this->decl.name;
}

data_off_t VarDeclList::GetTotalSize()
{
	data_off_t ret;
	if (this->empty()) {
		ret = 0;
	} else {
		ret = this->back().off + this->back().size;
	}
	assert(ret % 4 == 0);
	return ret;
}
void VarDeclList::Dump()
{
	printf("   var  %-8s %-8s %-10s %s\n", "offset", "size", "type", "name");
	for (auto &item: *this) {
		printf("   var  %08X %08X %-10s %s\n", (unsigned) item.off, (unsigned) item.size, ASTType::GetTypeName(item.decl.type.type), item.decl.name.c_str());
	}
	printf("   var  total %08X\n", GetTotalSize());
}

void VarDeclListVisitor::Visit(ASTVarDeclaration *node, int level)
{
	if (!list.Append(VarDeclItem {
		VarDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
		},
		list.GetTotalSize(),
		node->GetASTType()->GetTypeSize(),
	})) {
		MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate variable");
	}
}

data_off_t MethodDeclList::GetTotalSize()
{
	return this->size() * 4;
}

const std::string &MethodDeclItem::GetName() const
{
	return this->decl.name;
}
void MethodDeclList::Dump()
{
	for (auto &item: *this) {
		printf("  method: %08X %-10s %s():\n", (unsigned) item.off, ASTType::GetTypeName(item.decl.rettype.type), item.decl.name.c_str());
		printf("  >arg\n");
		item.decl.arg.Dump();
		printf("  >local\n");
		item.localvar.Dump();
	}
}

void MethodDeclListVisitor::Visit(ASTMethodDeclaration *node, int level)
{
	if (!list.Append(MethodDeclItem {
		MethodDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
			node->GetASTArgDeclarationList1()->GetVarDeclList(),
		},
		list.GetTotalSize(),
		node->GetASTVarDeclarationList()->GetVarDeclList(),
		std::dynamic_pointer_cast<ASTMethodDeclaration>(node->GetSharedPtr()),
	})) {
		MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate method");
	}

	for (auto &v: list.back().decl.arg) {
		if (list.back().localvar.Find(v.GetName()) != list.back().localvar.end()) {
			MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, v.GetName() + " exists in both local-var and method-arg");
			break;
		}
	}
}

const std::string &ClassInfoItem::GetName() const
{
	return name;
}
void ClassInfoItem::Dump()
{
	printf("class %s:\n", name.c_str());
	printf(" member-var:\n");
	var.Dump();
	printf(" method:\n");
	method.Dump();
}

void ClassInfoList::Dump()
{
	for (auto &item: *this) {
		item.Dump();
	}
}
void ClassInfoVisitor::Visit(ASTClassDeclaration *node, int level)
{
	if (!list.Append(ClassInfoItem{
		node->GetASTIdentifier()->id,
		node->GetASTVarDeclarationList()->GetVarDeclList(),
		node->GetASTMethodDeclarationList()->GetMethodDeclList(),
		std::dynamic_pointer_cast<ASTClassDeclaration>(node->GetSharedPtr()),
	})) {
		MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate class");
	}
}

// DataItem / DataBuffer
DataItem::DataItem()
{
}
std::shared_ptr<DataItem> DataItem::New()
{
	return std::shared_ptr<DataItem>(new DataItem());
}
std::shared_ptr<DataItem> DataItem::SetComment(const std::string &comment)
{
	this->comment = comment;
	return shared_from_this();
}
std::shared_ptr<DataItem> DataItem::AddU8(std::initializer_list<uint8_t> l)
{
	for (auto &v: l) {
		bytes.push_back(v);
	}
	return shared_from_this();
}
std::shared_ptr<DataItem> DataItem::AddU32(std::initializer_list<uint32_t> l)
{
	for (auto &v: l) {
		AddU8({
			(uint8_t)(v & 0xFF),
			(uint8_t)((v >> 8) & 0xFF),
			(uint8_t)((v >> 16) & 0xFF),
			(uint8_t)((v >> 24) & 0xFF),
		});
	}
	return shared_from_this();
}
std::shared_ptr<DataItem> DataItem::AddRel32(uint32_t val, RelocInfo::RelocType type, std::shared_ptr<DataItem> target)
{
	reloc.push_back(RelocInfo {
		(data_off_t) bytes.size(),
		type,
		target,
	});
	AddU32({val});
	return shared_from_this();
}
std::shared_ptr<DataItem> DataItem::AddString(const char *str)
{
	std::string c = "str: ";
	while (*str) {
		if (isprint(*str)) c += *str; else c += '.';
		AddU8({(uint8_t) *str++});
	}
	AddU8({0});
	SetComment(c);
	return shared_from_this();
}


std::shared_ptr<DataItem> DataBuffer::AppendItem(std::shared_ptr<DataItem> instr)
{
	list.push_back(instr);
	return instr;
}
std::shared_ptr<DataItem> DataBuffer::NewExternalSymbol(const std::string &name)
{
	std::shared_ptr<DataItem> marker = DataItem::New();
	extsym.push_back(std::make_pair(name, marker));
	return marker;
}
void DataBuffer::ProvideSymbol(const std::string &name)
{
	std::shared_ptr<DataItem> marker = DataItem::New();
	auto it = list.insert(list.end(), marker);
	sym.push_back(std::make_pair(name, it));
}

void DataBuffer::Dump()
{
	for (auto lstit = list.begin(); lstit != list.end(); lstit++) {
		auto &item = *lstit;
		for (auto &p: sym) {
			if (p.second == lstit) {
				printf(" <%s>:\n", p.first.c_str());
			}
		}

		if (item->bytes.empty()) continue;

		std::string bytesdump;
		for (auto &b: item->bytes) {
			char buf[4]; sprintf(buf, "%02X ", (unsigned) b);
			bytesdump += std::string(buf);
		}
		printf("  %08X: %-30s %s\n", item->off, bytesdump.c_str(), item->comment.c_str());

		// print reloc info
		if (!item->reloc.empty()) {
			bool flag = false;
			for (auto &r: item->reloc) {
				for (auto &p: extsym) { // FIXME: O(n^2)
					if (r.target == p.second) {
						printf("    reloc +%02X %s\n", r.off, p.first.c_str());
						flag = true;
					}
				}
			}
			if (!flag) printf("    reloc\n");
		}
	}
}


// Code Generator
std::string TypeInfo::GetName()
{
	std::string s(ASTType::GetTypeName(type));
	if (type == ASTType::VT_CLASS) {
		s += "(" + clsname + ")";
	}
	return s;
}

bool TypeInfo::operator == (const TypeInfo &r) const
{
	if (type == ASTType::VT_CLASS) {
		return type == r.type && clsname == r.clsname;
	} else {
		return type == r.type;
	}
}
bool TypeInfo::operator != (const TypeInfo &r) const
{
	return ! operator == (r);
}

CodeGen::CodeGen()
{
}
CodeGen *CodeGen::Instance()
{
	static CodeGen inst;
	return &inst;
}

TypeInfo CodeGen::PopType()
{
	TypeInfo r = varstack.back();
	varstack.pop_back();
	return r;
}
void CodeGen::PopAndCheckType(const yyltype &loc, TypeInfo tinfo)
{
	if (varstack.empty()) {
		MiniJavaC::Instance()->ReportError(loc, "internal error: stack empty");
		return;
	}
	TypeInfo &pinfo = varstack.back();
	
	if (tinfo != pinfo) {
		std::string msg = "type mismatch: " + pinfo.GetName() + ", expected " + tinfo.GetName();
		MiniJavaC::Instance()->ReportError(loc, msg);
	}

	varstack.pop_back();
}

void CodeGen::PushType(TypeInfo tinfo)
{
	varstack.push_back(tinfo);
}

void CodeGen::Visit(ASTStatement *node, int level)
{
	code.AppendItem(DataItem::New()->AddU8({0xCC})->SetComment("ERROR: unhandled statement"));
	printf("unhandled: %s\n", typeid(*node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "internal error: unhandled statement");
	VisitChildren(node, level);
}
void CodeGen::Visit(ASTExpression *node, int level)
{
	code.AppendItem(DataItem::New()->AddU8({0xCC})->SetComment("ERROR: unhandled statement"));
	printf("unhandled: %s\n", typeid(*node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "internal error: unhandled expression");
	VisitChildren(node, level);
}



// statment
//virtual void Visit(ASTArrayAssignStatement *node, int level);
//virtual void Visit(ASTAssignStatement *node, int level);
void CodeGen::Visit(ASTPrintlnStatement *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, TypeInfo { ASTType::VT_INT });

	auto fmtstr = rodata.AppendItem(DataItem::New()->AddString("%d\n"));
	code.AppendItem(DataItem::New()->AddU8({0x68})->AddRel32(0, RelocInfo::RELOC_ABS, fmtstr)->SetComment("PUSH fmtstr"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.printf"))->SetComment("CALL printf"));
	code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
}
//virtual void Visit(ASTWhileStatement *node, int level);
//virtual void Visit(ASTIfElseStatement *node, int level);
//virtual void Visit(ASTBlockStatement *node, int level);

// expression
void CodeGen::Visit(ASTIdentifier *node, int level)
{
	auto v = GetLocalVar(node->id);
	if (v.second.type != ASTType::VT_UNKNOWN) {
		assert(v.first.second % 4 == 0);
		for (data_off_t i = v.first.second - 4; i >= 0; i -= 4) {
			// push [ebp+(off+i)]
			code.AppendItem(DataItem::New()->AddU8({0xFF, 0xB5})->AddU32({(uint32_t)(v.first.first + i)})->SetComment("load local-var " + node->id));
		}
	} else {
		v = GetMemberVar(node->id);
		if (v.second.type != ASTType::VT_UNKNOWN) {
			LoadThisToEAX();
			assert(v.first.second % 4 == 0);
			for (data_off_t i = v.first.second - 4; i >= 0; i -= 4) {
				// push [eax+(off+i)]
				code.AppendItem(DataItem::New()->AddU8({0xFF, 0xB0})->AddU32({(uint32_t)(v.first.first + i)})->SetComment("load member-var " + node->id));
			}
		} else {
			MiniJavaC::Instance()->ReportError(node->loc, "undeclared identifier " + node->id);
		}
	}
	PushType(v.second);
}
void CodeGen::Visit(ASTBoolean *node, int level)
{
	PushType(TypeInfo { ASTType::VT_BOOLEAN });
	code.AppendItem(DataItem::New()->AddU8({0x6A})->AddU8({(uint8_t)node->val})->SetComment("PUSH ast_boolean"));
}
void CodeGen::Visit(ASTNumber *node, int level)
{
	PushType(TypeInfo { ASTType::VT_INT });
	code.AppendItem(DataItem::New()->AddU8({0x68})->AddU32({(uint32_t)node->val})->SetComment("PUSH ast_number"));
}
void CodeGen::Visit(ASTBinaryExpression *node, int level)
{
	GenerateCodeForASTNode(node->GetLeftASTExpression());
	GenerateCodeForASTNode(node->GetRightASTExpression());

	TypeInfo ltype, rtype, restype; // l/r operand type, result type
	switch (node->op) {
		case TOK_LAND:
			ltype = rtype = restype = TypeInfo { ASTType::VT_BOOLEAN };
			break;
		case TOK_LT: // less then
			ltype = rtype = TypeInfo { ASTType::VT_INT };
			restype = TypeInfo { ASTType::VT_BOOLEAN };
			break;
		case TOK_ADD:
		case TOK_SUB:
		case TOK_MUL:
			ltype = rtype = restype = TypeInfo { ASTType::VT_INT };
			break;
		case TOK_LS: // subscript
			ltype = TypeInfo { ASTType::VT_INTARRAY };
			rtype = restype = TypeInfo { ASTType::VT_INT };

			break;
		default: panic();
	}
	PopAndCheckType(node->GetRightASTExpression()->loc, rtype);
	PopAndCheckType(node->GetLeftASTExpression()->loc, ltype);

	switch (node->op) {
		case TOK_LAND:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x21, 0x04, 0xE4})->SetComment("AND [ESP],EAX"));
			break;
		case TOK_LT:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x39, 0x04, 0xE4})->SetComment("CMP [ESP],EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x0F, 0x9C, 0xC0})->SetComment("SETL AL"));
			code.AppendItem(DataItem::New()->AddU8({0x0F, 0xB6, 0xC0})->SetComment("MOVZX EAX,AL"));
			code.AppendItem(DataItem::New()->AddU8({0x89, 0x04, 0xE4})->SetComment("MOV [ESP],EAX"));
			break;
		case TOK_ADD:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x01, 0x04, 0xE4})->SetComment("ADD [ESP],EAX"));
			break;
		case TOK_SUB:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x29, 0x04, 0xE4})->SetComment("SUB [ESP],EAX"));
			break;
		case TOK_MUL:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0xF7, 0x2C, 0xE4})->SetComment("IMUL [ESP]"));
			code.AppendItem(DataItem::New()->AddU8({0x89, 0x04, 0xE4})->SetComment("MOV [ESP],EAX"));
			break;
		case TOK_LS:
			code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
			code.AppendItem(DataItem::New()->AddU8({0x59})->SetComment("POP ECX"));
			code.AppendItem(DataItem::New()->AddU8({0x8B, 0x04, 0x81})->SetComment("MOV EAX,[EAX*4+ECX]"));
			code.AppendItem(DataItem::New()->AddU8({0x89, 0x04, 0xE4})->SetComment("MOV [ESP],EAX"));
			break;
		default: panic();
	}

	PushType(restype);
}
void CodeGen::Visit(ASTUnaryExpression *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	switch (node->op) {
		case TOK_NOT:
			PopAndCheckType(node->GetASTExpression()->loc, TypeInfo { ASTType::VT_BOOLEAN });
			code.AppendItem(DataItem::New()->AddU8({0x83, 0x34, 0xE4, 0x01})->SetComment("XOR [ESP],1"));
			PushType(TypeInfo { ASTType::VT_BOOLEAN });
			break;
		case TOK_LP:
			// nothing to do
			break;
		default: panic();
	};
}

void CodeGen::Visit(ASTArrayLengthExpression *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, TypeInfo { ASTType::VT_INTARRAY });
	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
	PushType(TypeInfo { ASTType::VT_INT });
}

void CodeGen::Visit(ASTFunctionCallExpression *node, int level)
{
	class MethodArgVisitor : public ASTNodeVisitor {
	public:
		std::vector<std::shared_ptr<ASTNode> > arglist;
		virtual void Visit(ASTExpression *node, int level) override
		{
			arglist.push_back(node->GetSharedPtr());
		}
	};

	MethodArgVisitor v;
	node->GetASTArgExpressionList1()->Accept(v);
	std::reverse(v.arglist.begin(), v.arglist.end());

	for (auto &argexpr: v.arglist) {
		GenerateCodeForASTNode(argexpr);
	}
	
	GenerateCodeForASTNode(node->GetASTExpression());
	TypeInfo cls = PopType();
	VarDeclList *marglist = nullptr;
	data_off_t vtbloff;
	TypeInfo rtype;

	if (cls.type == ASTType::VT_CLASS) {
		auto cit = clsinfo.Find(cls.clsname);
		if (cit != clsinfo.end()) {
			auto mit = cit->method.Find(node->GetASTIdentifier()->id);
			if (mit != cit->method.end()) {
				marglist = &mit->decl.arg;
				vtbloff = mit->off;
				rtype = mit->decl.rettype;
			} else {
				MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "no such method");
			}
		} else {
			MiniJavaC::Instance()->ReportError(node->GetASTExpression()->loc, "no such class");
		}
	} else {
		MiniJavaC::Instance()->ReportError(node->GetASTExpression()->loc, "not a class");
	}

	if (marglist && marglist->size() == v.arglist.size()) {
		for (auto it = marglist->rbegin(); it != marglist->rend(); it++) {	
			PopAndCheckType((*(v.arglist.rbegin() + (it - marglist->rbegin())))->loc, it->decl.type);
		}
		code.AppendItem(DataItem::New()->AddU8({0x8B, 0x04, 0xE4})->SetComment("MOV EAX,[ESP] (eax=this)"));
		code.AppendItem(DataItem::New()->AddU8({0x8B, 0x00})->SetComment("MOV EAX,[EAX] (eax=vfptr)"));
		code.AppendItem(DataItem::New()->AddU8({0xFF, 0x90})->AddU32({(uint32_t)vtbloff})->SetComment("CALL [EAX+vtbloff] (eax=vfptr)"));
		code.AppendItem(DataItem::New()->AddU8({0x81, 0xC4})->AddU32({(uint32_t)((v.arglist.size() + 1) * 4)})->SetComment("ADD ESP,argsize"));
		code.AppendItem(DataItem::New()->AddU8({0x50})->SetComment("PUSH EAX"));

		PushType(rtype);
	} else {
		for (auto &t: v.arglist) {
			PopType();
		}
		MiniJavaC::Instance()->ReportError(node->GetASTArgExpressionList1()->loc, "arg number mismatch");
		PushType(TypeInfo { ASTType::VT_UNKNOWN });
	}
}
void CodeGen::Visit(ASTThisExpression *node, int level)
{
	if (cur_cls) {
		LoadThisToEAX();
		code.AppendItem(DataItem::New()->AddU8({0x50})->SetComment("PUSH EAX"));
		PushType(TypeInfo { ASTType::VT_CLASS, cur_cls->name });
	} else {
		MiniJavaC::Instance()->ReportError(node->loc, "invalid use of this");
		PushType(TypeInfo { ASTType::VT_UNKNOWN });
	}
}
void CodeGen::Visit(ASTNewIntArrayExpression *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, TypeInfo { ASTType::VT_INT });
	code.AppendItem(DataItem::New()->AddU8({0x6A, 0x04})->SetComment("PUSH 4"));
	code.AppendItem(DataItem::New()->AddU8({0xFF, 0x74, 0xE4, 0x04})->SetComment("PUSH [ESP+4]"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.calloc"))->SetComment("CALL calloc"));
	code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
	code.AppendItem(DataItem::New()->AddU8({0x50})->SetComment("PUSH EAX"));
	PushType(TypeInfo { ASTType::VT_INTARRAY });
}
void CodeGen::Visit(ASTNewExpression *node, int level)
{
	auto clsname = node->GetASTIdentifier()->id;
	auto it = clsinfo.Find(clsname);
	if (it != clsinfo.end()) {
		data_off_t clssize = it->var.GetTotalSize() + 4;
		
		code.AppendItem(DataItem::New()->AddU8({0x68})->AddU32({(uint32_t)clssize})->SetComment("PUSH clssize"));
		code.AppendItem(DataItem::New()->AddU8({0x6A, 0x01})->SetComment("PUSH 1"));
		code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.calloc"))->SetComment("CALL calloc"));
		code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
		code.AppendItem(DataItem::New()->AddU8({0x50})->SetComment("PUSH EAX"));
		code.AppendItem(DataItem::New()->AddU8({0xC7, 0x00})->AddRel32(0, RelocInfo::RELOC_ABS, code.NewExternalSymbol(clsname + ".$vfptr"))->SetComment("MOV [EAX],vfptr"));
	
		PushType(TypeInfo { ASTType::VT_CLASS, clsname });
	} else {
		MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "undeclared class " + clsname);
		PushType(TypeInfo { ASTType::VT_UNKNOWN });
	}
}


void CodeGen::LoadThisToEAX()
{
	code.AppendItem(DataItem::New()->AddU8({0x8B, 0x45, 0x08})->SetComment("MOV EAX,[EBP+8] (load this)"));
}

std::pair<std::pair<data_off_t, data_off_t>, TypeInfo> CodeGen::GetLocalVar(const std::string &name)
{	
	if (cur_cls && cur_method) {
		auto lvar = cur_method->localvar.Find(name);
		if (lvar != cur_method->localvar.end()) {
			return std::make_pair(std::make_pair(-cur_method->localvar.GetTotalSize() + lvar->off, lvar->size), lvar->decl.type);
		}

		auto avar = cur_method->decl.arg.Find(name);
		if (avar != cur_method->decl.arg.end()) {
			return std::make_pair(std::make_pair(0xC + avar->off, avar->size), avar->decl.type);
		}
	}
	return std::make_pair(std::make_pair(0, 0), TypeInfo {ASTType::VT_UNKNOWN});
}
std::pair<std::pair<data_off_t, data_off_t>, TypeInfo> CodeGen::GetMemberVar(const std::string &name)
{
	if (cur_cls && cur_method) {
		auto mvar = cur_cls->var.Find(name);
		if (mvar != cur_cls->var.end()) {
			return std::make_pair(std::make_pair(0x4 + mvar->off, mvar->size), mvar->decl.type);
		}
	}
	return std::make_pair(std::make_pair(0, 0), TypeInfo {ASTType::VT_UNKNOWN});
}

void CodeGen::GenerateCodeForASTNode(std::shared_ptr<ASTNode> node)
{
	node->ASTNode::Accept(*this);
}
void CodeGen::GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls)
{
	cur_cls = nullptr;
	cur_method = nullptr;
	code.ProvideSymbol("main");
	GenerateCodeForASTNode(maincls->GetASTStatement());
	code.AppendItem(DataItem::New()->AddU8({0x6A, 0x00})->SetComment("PUSH 0"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.exit"))->SetComment("CALL exit"));
}
void CodeGen::GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method)
{
	cur_cls = &cls;
	cur_method = &method;
	
	code.ProvideSymbol(cls.GetName() + "." + method.GetName());

	code.AppendItem(DataItem::New()->AddU8({0x55})->SetComment("PUSH EBP"));
	code.AppendItem(DataItem::New()->AddU8({0x8B, 0xEC})->SetComment("MOV EBP,ESP"));
	
	data_off_t localsize = method.localvar.GetTotalSize();
	assert(localsize % 4 == 0);
	for (data_off_t i = 0; i < localsize / 4; i++) {
		code.AppendItem(DataItem::New()->AddU8({0x6A, 0x00})->SetComment("PUSH 0"));
	}

	GenerateCodeForASTNode(method.ptr->GetASTStatementList());


	GenerateCodeForASTNode(method.ptr->GetASTExpression());
	PopAndCheckType(method.ptr->GetASTExpression()->loc, method.decl.rettype);
	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
	
	code.AppendItem(DataItem::New()->AddU8({0xC9})->SetComment("LEAVE"));
	code.AppendItem(DataItem::New()->AddU8({0xC3})->SetComment("RETN"));
}
void CodeGen::GenerateCode()
{
	printf("[*] Generating type information ...\n");
	clsinfo = MiniJavaC::Instance()->goal->GetClassInfoList();

	printf("[*[ Generating code for main() ...\n");
	GenerateCodeForMainMethod(MiniJavaC::Instance()->goal->GetASTMainClass());

	for (auto &cls: clsinfo) {
		printf("[*] Generating code for class %s ...\n", cls.GetName().c_str());
		for (auto &method: cls.method) {
			printf("  [*] Generating code for %s::%s() ...\n", cls.GetName().c_str(), method.GetName().c_str());
			GenerateCodeForClassMethod(cls, method);
		}
	}

	printf(".code:\n");
	code.Dump();
	printf(".rodata:\n");
	rodata.Dump();
	printf(".data:\n");
	data.Dump();
}
