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
		uint8_t b[4]; memcpy(b, &v, 4);
		AddU8({b[0], b[1], b[2], b[3]});
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
std::shared_ptr<DataItem> DataItem::SetAlign(data_off_t align, uint8_t fill)
{
	this->align = align;
	this->align_fill = fill;
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
	extsym.push_back(std::make_pair(name, std::make_pair(marker, false)));
	return marker;
}
void DataBuffer::ProvideSymbol(const std::string &name)
{
	std::shared_ptr<DataItem> marker = DataItem::New();
	auto it = list.insert(list.end(), marker);
	sym.push_back(std::make_pair(name, std::make_pair(&list, it)));
}

data_off_t DataBuffer::CalcOffset(data_off_t base)
{
	base_addr = base;
	for (auto &item: list) {
		base = ROUNDUP(base, item->align);
		item->off = base;
		base += item->bytes.size();
	}
	return end_addr = base;
}
void DataBuffer::DoRelocate(data_off_t rva_base)
{
	for (auto &item: list) {
		for (auto &r: item->reloc) {
			// assume 32bit
			data_off_t addr = item->off/* + r.off*/;
			data_off_t target = r.target->off;

			data_off_t olddata; memcpy(&olddata, item->bytes.data() + r.off, 4);
			data_off_t newdata;

			switch (r.type) {
				case RelocInfo::RELOC_ABS32:
					newdata = target;
					break;
				case RelocInfo::RELOC_REL32:
					newdata = target - (addr + olddata);
					break;
				case RelocInfo::RELOC_RVA32:
					newdata = target - rva_base;
					break;
				default: panic();
			}

			memcpy(item->bytes.data() + r.off, &newdata, 4);
		}
	}
}
void DataBuffer::ReduceSymbols(const std::vector<DataBuffer *> buffers)
{
	std::map<std::string, std::pair<std::list<std::shared_ptr<DataItem> > *, std::list<std::shared_ptr<DataItem> >::iterator > > allsym;

	for (auto &b: buffers) {
		for (auto &s: b->sym) {
			if (!allsym.insert(s).second) {
				MiniJavaC::Instance()->ReportError("duplicate symbol: " + s.first);
			}
		}
	}

	for (auto &b: buffers) {
		for (auto &s: b->extsym) {
			if (!s.second.second) {
				auto it = allsym.find(s.first);
				if (it != allsym.end()) {
					it->second.first->insert(it->second.second, s.second.first);
					s.second.second = true;
				} else {
					MiniJavaC::Instance()->ReportError("unresolved external symbol: " + s.first);
				}
			}
		}
	}
}

void DataBuffer::Dump()
{
	for (auto lstit = list.begin(); lstit != list.end(); lstit++) {
		auto &item = *lstit;
		for (auto &p: sym) {
			if (p.second.second == lstit) {
				printf(" <%s>:\n", p.first.c_str());
			}
		}

		if (item->bytes.empty()) continue;

		std::string bytesdump;
		for (auto &b: item->bytes) {
			char buf[4]; sprintf(buf, "%02X ", (unsigned) b);
			bytesdump += std::string(buf);
		}
		printf("  %08X: %-30s %s\n", (unsigned) item->off, bytesdump.c_str(), item->comment.c_str());

		// print reloc info
		if (!item->reloc.empty()) {
			printf("    reloc ");
			for (auto &r: item->reloc) {
				for (auto &p: extsym) { // FIXME: O(n^2)
					if (r.target == p.second.first) {
						printf("(+%02X %s)", r.off, p.first.c_str());
					}
				}
				printf(":%08X ", r.target->off);
			}
			printf("\n");
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

void CodeGen::AssertTypeEmpty(const yyltype &loc)
{
	if (!varstack.empty()) {
		MiniJavaC::Instance()->ReportError(loc, "internal error: assert failed, stack not empty");
		return;
	}
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
void CodeGen::Visit(ASTArrayAssignStatement *node, int level)
{
	GenerateCodeForASTNode(node->GetASTIdentifier());
	PopAndCheckType(node->GetASTIdentifier()->loc, TypeInfo{ASTType::VT_INTARRAY});

	GenerateCodeForASTNode(node->GetSubscriptASTExpression());
	PopAndCheckType(node->GetSubscriptASTExpression()->loc, TypeInfo{ASTType::VT_INT});

	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, TypeInfo{ASTType::VT_INT});

	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x59})->SetComment("POP ECX"));
	code.AppendItem(DataItem::New()->AddU8({0x5A})->SetComment("POP EDX"));
	code.AppendItem(DataItem::New()->AddU8({0x89, 0x04, 0x8A})->SetComment("MOV [ECX*4+EDX],EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
}
void CodeGen::Visit(ASTAssignStatement *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	auto v = GetLocalVar(node->GetASTIdentifier()->id);
	if (v.second.type != ASTType::VT_UNKNOWN) {
		PopAndCheckType(node->GetASTIdentifier()->loc, v.second);
		assert(v.first.second % 4 == 0);
		for (data_off_t i = 0; i < v.first.second; i += 4) {
			// pop [ebp+(off+i)]
			code.AppendItem(DataItem::New()->AddU8({0x8F, 0x85})->AddU32({(uint32_t)(v.first.first + i)})->SetComment("store local-var " + node->GetASTIdentifier()->id));
		}
	} else {
		v = GetMemberVar(node->GetASTIdentifier()->id);
		if (v.second.type != ASTType::VT_UNKNOWN) {
			PopAndCheckType(node->GetASTIdentifier()->loc, v.second);
			LoadThisToEAX();
			assert(v.first.second % 4 == 0);
			for (data_off_t i = 0; i < v.first.second; i += 4) {
				// pop [eax+(off+i)]
				code.AppendItem(DataItem::New()->AddU8({0x8F, 0x80})->AddU32({(uint32_t)(v.first.first + i)})->SetComment("store member-var " + node->GetASTIdentifier()->id));
			}
		} else {
			MiniJavaC::Instance()->ReportError(node->loc, "undeclared identifier " + node->GetASTIdentifier()->id);
		}
	}
}
void CodeGen::Visit(ASTPrintlnStatement *node, int level)
{
	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, TypeInfo { ASTType::VT_INT });

	auto fmtstr = data.AppendItem(DataItem::New()->AddString("%d\n"));
	code.AppendItem(DataItem::New()->AddU8({0x68})->AddRel32(0, RelocInfo::RELOC_ABS32, fmtstr)->SetComment("PUSH fmtstr"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL32, code.NewExternalSymbol("IMP$msvcrt.printf"))->SetComment("CALL printf"));
	code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
}
void CodeGen::Visit(ASTWhileStatement *node, int level)
{
	auto beginmarker = DataItem::New();
	auto endmarker = DataItem::New();

	code.AppendItem(beginmarker);
	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, (TypeInfo { ASTType::VT_BOOLEAN }));
	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x85, 0xC0})->SetComment("TEST EAX,EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x0F, 0x84})->AddRel32(0x6, RelocInfo::RELOC_REL32, endmarker)->SetComment("JZ end-marker"));

	GenerateCodeForASTNode(node->GetASTStatement());
	code.AppendItem(DataItem::New()->AddU8({0xE9})->AddRel32(0x5, RelocInfo::RELOC_REL32, beginmarker)->SetComment("JMP begin-marker"));
	
	code.AppendItem(endmarker);
}
void CodeGen::Visit(ASTIfElseStatement *node, int level)
{
	auto endmarker = DataItem::New();
	auto elsemarker = DataItem::New();

	GenerateCodeForASTNode(node->GetASTExpression());
	PopAndCheckType(node->GetASTExpression()->loc, (TypeInfo { ASTType::VT_BOOLEAN }));
	code.AppendItem(DataItem::New()->AddU8({0x58})->SetComment("POP EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x85, 0xC0})->SetComment("TEST EAX,EAX"));
	code.AppendItem(DataItem::New()->AddU8({0x0F, 0x84})->AddRel32(0x6, RelocInfo::RELOC_REL32, elsemarker)->SetComment("JZ else-marker"));

	GenerateCodeForASTNode(node->GetThenASTStatement());

	code.AppendItem(DataItem::New()->AddU8({0xE9})->AddRel32(0x5, RelocInfo::RELOC_REL32, endmarker)->SetComment("JMP end-marker"));
	code.AppendItem(elsemarker);

	GenerateCodeForASTNode(node->GetElseASTStatement());
	code.AppendItem(endmarker);
}
void CodeGen::Visit(ASTBlockStatement *node, int level)
{
	VisitChildren(node, level);
}

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
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL32, code.NewExternalSymbol("IMP$msvcrt.calloc"))->SetComment("CALL calloc"));
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
		code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL32, code.NewExternalSymbol("IMP$msvcrt.calloc"))->SetComment("CALL calloc"));
		code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
		code.AppendItem(DataItem::New()->AddU8({0x50})->SetComment("PUSH EAX"));
		code.AppendItem(DataItem::New()->AddU8({0xC7, 0x00})->AddRel32(0, RelocInfo::RELOC_ABS32, code.NewExternalSymbol(clsname + ".$vfptr"))->SetComment("MOV [EAX],vfptr"));
	
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
	code.ProvideSymbol("$ENTRY");
	GenerateCodeForASTNode(maincls->GetASTStatement());
	code.AppendItem(DataItem::New()->AddU8({0x6A, 0x00})->SetComment("PUSH 0"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL32, code.NewExternalSymbol("IMP$msvcrt.exit"))->SetComment("CALL exit"));
	AssertTypeEmpty(maincls->GetASTStatement()->loc);
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
	AssertTypeEmpty(method.ptr->GetASTExpression()->loc);
}
void CodeGen::GenerateVtblForClass(ClassInfoItem &cls)
{
	rodata.ProvideSymbol(cls.GetName() + ".$vfptr");
	rodata.AppendItem(DataItem::New()->SetAlign(4));
	for (auto &method: cls.method) {
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_ABS32, rodata.NewExternalSymbol(cls.GetName() + "." + method.GetName())));
	}
}
void CodeGen::AddImportEntry(const std::string &dllname, const std::vector<std::string> &funclist)
{
	dllinfo.push_back(std::make_pair(dllname, funclist));
	for (auto &func: funclist) {
		code.ProvideSymbol("IMP$" + dllname + "." + func);
		code.AppendItem(DataItem::New()->AddU8({0xFF, 0x25})->AddRel32(0, RelocInfo::RELOC_ABS32, code.NewExternalSymbol("IAT.FIRSTTHUNK$" + dllname + "." + func))->SetComment(dllname + "." + func));
	}
}
void CodeGen::MakeIAT()
{
	rodata.AppendItem(DataItem::New()->SetAlign(4));
	
	// make FirstThunk
	for (auto &dllitem: dllinfo) {
		std::string &dllname = dllitem.first;
		rodata.ProvideSymbol("IAT.FIRSTTHUNK.DLL$" + dllname);
		for (auto &func: dllitem.second) {
			rodata.ProvideSymbol("IAT.FIRSTTHUNK$" + dllname + "." + func);
			rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.HINTNAME$" + dllname + "." + func))->SetComment(dllname + "." + func));
		}
	}

	// make IMAGE_IMPORT_DESCRIPTOR
	rodata.ProvideSymbol("$IAT");
	for (auto &dllitem: dllinfo) {
		std::string &dllname = dllitem.first;
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.HINTNAME.DLL$" + dllname))->SetComment("Characteristics"));
		rodata.AppendItem(DataItem::New()->AddU32({0})->SetComment("TimeDateStamp"));
		rodata.AppendItem(DataItem::New()->AddU32({0})->SetComment("ForwarderChain"));
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.NAME.DLL$" + dllname))->SetComment("Name"));
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.FIRSTTHUNK.DLL$" + dllname))->SetComment("FirstThunk"));
	}
	rodata.AppendItem(DataItem::New()->AddU32({0, 0, 0, 0, 0}));

	// make HINTNAME and IMAGE_IMPORT_BY_NAME and DLL NAME
	for (auto &dllitem: dllinfo) {
		std::string &dllname = dllitem.first;
		rodata.AppendItem(DataItem::New()->SetAlign(4));

		// HINTNAME
		rodata.ProvideSymbol("IAT.HINTNAME.DLL$" + dllname);
		for (auto &func: dllitem.second) {
			rodata.ProvideSymbol("IAT.HINTNAME$" + dllname + "." + func);
			rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.IMPBYNAME$" + dllname + "." + func))->SetComment(dllname + "." + func));
		}
		rodata.AppendItem(DataItem::New()->AddU32({0}));

		// IMAGE_IMPORT_BY_NAME
		for (auto &func: dllitem.second) {
			rodata.ProvideSymbol("IAT.IMPBYNAME$" + dllname + "." + func);
			rodata.AppendItem(DataItem::New()->SetAlign(2)->AddU8({0, 0}));
			rodata.AppendItem(DataItem::New()->AddString(func.c_str()));
		}

		// DLL NAME
		rodata.ProvideSymbol("IAT.NAME.DLL$" + dllname);
		rodata.AppendItem(DataItem::New()->SetAlign(0x10)->AddString((dllname + ".dll").c_str()));
	}
}
void CodeGen::MakeEXE()
{
	
}
void CodeGen::Link()
{
	const data_off_t pe_base = 0x00400000;
	const data_off_t sect_align = 0x1000;
	data_off_t base = pe_base + 0x1000;

	std::vector<DataBuffer *> sections{&code, &rodata, &data};

	printf(" [*] Processing symbols ...\n");
	DataBuffer::ReduceSymbols(sections);

	printf(" [*] Calc address ...\n");
	for (auto &sect: sections) {
		base = sect->CalcOffset(base);
		base = ROUNDUP(base, sect_align);
	}

	printf(" [*] Relocating ...\n");
	for (auto &sect: sections) {
		sect->DoRelocate(pe_base);
	}

	printf(" [*] Making EXE ...\n");
	MakeEXE();
}

void CodeGen::DumpSections()
{
	printf(".code:\n");
	code.Dump();
	printf(".rodata:\n");
	rodata.Dump();
	printf(".data:\n");
	data.Dump();
}
void CodeGen::GenerateCode()
{
	printf("[*] Generating type information ...\n");
	clsinfo = MiniJavaC::Instance()->goal->GetClassInfoList();

	printf("[*] Generating code for main() ...\n");
	GenerateCodeForMainMethod(MiniJavaC::Instance()->goal->GetASTMainClass());

	for (auto &cls: clsinfo) {
		printf("[*] Generating code for class %s ...\n", cls.GetName().c_str());
		for (auto &method: cls.method) {
			printf("  [*] Generating code for %s::%s() ...\n", cls.GetName().c_str(), method.GetName().c_str());
			GenerateCodeForClassMethod(cls, method);
		}
	}

	for (auto &cls: clsinfo) {
		printf("[*] Generating virtual function table for class %s ...\n", cls.GetName().c_str());
		GenerateVtblForClass(cls);
	}

	printf("[*] Adding DLL import table ...\n");
	AddImportEntry("msvcrt", {"printf", "calloc", "exit"});
	MakeIAT();


	printf("[*] Linking ...\n");
	Link();


	DumpSections();
}
