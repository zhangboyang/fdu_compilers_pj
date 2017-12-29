#include "common.h"
#include "minijavac.tab.h"

bool MethodDecl::operator == (const MethodDecl &r) const
{
	if (name != r.name) return false;
	if (rettype != r.rettype) return false;
	if (arg.size() != r.arg.size()) return false;
	for (size_t i = 0; i < arg.size(); i++) {
		if (arg[i].decl.type != r.arg[i].decl.type) return false;
	}
	return true;
}

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

VarDeclListVisitor::VarDeclListVisitor(VarDeclList base) : list(base)
{
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

MethodDeclListVisitor::MethodDeclListVisitor(MethodDeclList base, const std::string &clsname) : list(base), clsname(clsname)
{
}
void MethodDeclListVisitor::Visit(ASTMethodDeclaration *node, int level)
{
	MethodDeclItem new_item {
		MethodDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
			node->GetASTArgDeclarationList1()->GetVarDeclList(VarDeclList()),
		},
		list.GetTotalSize(),
		node->GetASTVarDeclarationList()->GetVarDeclList(VarDeclList()),
		clsname,
		std::dynamic_pointer_cast<ASTMethodDeclaration>(node->GetSharedPtr()),
	};


	auto it = list.Find(new_item.GetName());
	if (it == list.end()) {
		list.Append(new_item);
	} else {
		if (it->clsname != new_item.clsname) {
			// override method
			if (it->decl == new_item.decl) {
				new_item.off = it->off;
				*it = new_item;
			} else {
				MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "different method prototype");
			}
		} else {
			MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate method");
		}
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
		node->GetASTVarDeclarationList()->GetVarDeclList(VarDeclList()),
		node->GetASTMethodDeclarationList()->GetMethodDeclList(MethodDeclList(), node->GetASTIdentifier()->id),
		std::string(),
	})) {
		MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate class");
	}
}
void ClassInfoVisitor::Visit(ASTDerivedClassDeclaration *node, int level)
{
	auto it = list.Find(node->GetBaseASTIdentifier()->id);
	if (it != list.end()) {
		if (!list.Append(ClassInfoItem{
			node->GetASTIdentifier()->id,
			node->GetASTVarDeclarationList()->GetVarDeclList(it->var),
			node->GetASTMethodDeclarationList()->GetMethodDeclList(it->method, node->GetASTIdentifier()->id),
			it->GetName(),
		})) {
			MiniJavaC::Instance()->ReportError(node->GetASTIdentifier()->loc, "duplicate class");
		}
	} else {
		MiniJavaC::Instance()->ReportError(node->GetBaseASTIdentifier()->loc, "no such class");
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

std::vector<uint8_t> DataBuffer::GetContent()
{
	std::vector<uint8_t> r;

	data_off_t cursor = base_addr;
	for (auto &item: list) {
		while (cursor < item->off) {
			r.push_back(item->align_fill);
			cursor++;
		}
		r.insert(r.end(), item->bytes.begin(), item->bytes.end());
		cursor += item->bytes.size();
	}

	return r;
}

void DataBuffer::DoRelocate()
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
					newdata = CodeGen::ToRVA(target);
					break;
				default: panic();
			}

			memcpy(item->bytes.data() + r.off, &newdata, 4);
		}
	}
}
data_off_t DataBuffer::GetSymbol(const std::string &symname)
{
	for (auto &s: sym) {
		if (s.first == symname) {
			return (*s.second.second)->off;
		}
	}
	return 0;
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

void DataBuffer::Dump(FILE *fp)
{
	for (auto lstit = list.begin(); lstit != list.end(); lstit++) {
		auto &item = *lstit;
		for (auto &p: sym) {
			if (p.second.second == lstit) {
				fprintf(fp, " <%s>:\n", p.first.c_str());
			}
		}

		if (item->bytes.empty()) continue;

		std::string bytesdump;
		for (auto &b: item->bytes) {
			char buf[5]; sprintf(buf, "%02X ", (unsigned) b);
			bytesdump += std::string(buf);
		}
		fprintf(fp, "  %08X: %-30s %s\n", (unsigned) item->off, bytesdump.c_str(), item->comment.c_str());

		// print reloc info
		if (!item->reloc.empty()) {
			fprintf(fp, "    reloc ");
			for (auto &r: item->reloc) {
				for (auto &p: extsym) { // FIXME: O(n^2)
					if (r.target == p.second.first) {
						fprintf(fp, "(+%02X %s)", r.off, p.first.c_str());
					}
				}
				fprintf(fp, ":%08X ", r.target->off);
			}
			fprintf(fp, "\n");
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
bool TypeInfo::CanCastTo(const TypeInfo &r) const
{
	if (type != ASTType::VT_CLASS) return false;
	if (r.type != ASTType::VT_CLASS) return false;
	std::string curcls = clsname;
	while (1) {
		auto &clsinfo = CodeGen::Instance()->clsinfo;
		if (curcls == r.clsname) return true;
		if (curcls == "") return false;
		auto it = clsinfo.Find(curcls);
		if (it == clsinfo.end()) return false;
		curcls = it->base;
	}
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
	
	if (tinfo != pinfo && !pinfo.CanCastTo(tinfo)) {
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
		for (auto it = marglist->begin(); it != marglist->end(); it++) {	
			PopAndCheckType((*(v.arglist.rbegin() + (it - marglist->begin())))->loc, it->decl.type);
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
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_ABS32, rodata.NewExternalSymbol(method.clsname + "." + method.GetName())));
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
	rodata.ProvideSymbol("$IAT");
	for (auto &dllitem: dllinfo) {
		std::string &dllname = dllitem.first;
		rodata.ProvideSymbol("IAT.FIRSTTHUNK.DLL$" + dllname);
		for (auto &func: dllitem.second) {
			rodata.ProvideSymbol("IAT.FIRSTTHUNK$" + dllname + "." + func);
			rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.IMPBYNAME$" + dllname + "." + func))->SetComment(dllname + "." + func));
		}
	}
	rodata.ProvideSymbol("$IAT.END");

	// make IMAGE_IMPORT_DESCRIPTOR
	rodata.ProvideSymbol("$IMPORT_DIRECTORY");
	for (auto &dllitem: dllinfo) {
		std::string &dllname = dllitem.first;
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.HINTNAME.DLL$" + dllname))->SetComment("Characteristics"));
		rodata.AppendItem(DataItem::New()->AddU32({0})->SetComment("TimeDateStamp"));
		rodata.AppendItem(DataItem::New()->AddU32({0})->SetComment("ForwarderChain"));
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.NAME.DLL$" + dllname))->SetComment("Name"));
		rodata.AppendItem(DataItem::New()->AddRel32(0, RelocInfo::RELOC_RVA32, rodata.NewExternalSymbol("IAT.FIRSTTHUNK.DLL$" + dllname))->SetComment("FirstThunk"));
	}
	rodata.AppendItem(DataItem::New()->AddU32({0, 0, 0, 0, 0}));
	rodata.ProvideSymbol("$IMPORT_DIRECTORY.END");

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
			rodata.AppendItem(DataItem::New()->SetAlign(2));
			rodata.ProvideSymbol("IAT.IMPBYNAME$" + dllname + "." + func);
			rodata.AppendItem(DataItem::New()->AddU8({0, 0}));
			rodata.AppendItem(DataItem::New()->AddString(func.c_str()));
		}

		// DLL NAME
		rodata.AppendItem(DataItem::New()->SetAlign(0x10));
		rodata.ProvideSymbol("IAT.NAME.DLL$" + dllname);
		rodata.AppendItem(DataItem::New()->AddString((dllname + ".dll").c_str()));
	}
}
void CodeGen::MakeEXE()
{
	FILE *fp = fopen("out.exe", "wb");

	
	// dos header and dos stub
	fwrite(
		"\x4D\x5A\x90\x00\x03\x00\x00\x00\x04\x00\x00\x00\xFF\xFF\x00\x00"
		"\xB8\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00"
		"\x0E\x1F\xBA\x0E\x00\xB4\x09\xCD\x21\xB8\x01\x4C\xCD\x21\x54\x68"
		"\x69\x73\x20\x70\x72\x6F\x67\x72\x61\x6D\x20\x63\x61\x6E\x6E\x6F"
		"\x74\x20\x62\x65\x20\x72\x75\x6E\x20\x69\x6E\x20\x44\x4F\x53\x20"
		"\x6D\x6F\x64\x65\x2E\x0D\x0D\x0A\x24\x00\x00\x00\x00\x00\x00\x00",
		1, 0x80, fp);

	
	IMAGE_NT_HEADERS nthdr; memset(&nthdr, 0, sizeof(nthdr));
    PIMAGE_FILE_HEADER pfilehdr = &nthdr.FileHeader;
    PIMAGE_OPTIONAL_HEADER popthdr = &nthdr.OptionalHeader;
	PIMAGE_DATA_DIRECTORY ppedirectory = popthdr->DataDirectory;
	
	assert(PE_FILEALIGN == PE_SECTIONALIGN);
	nthdr.Signature = 0x4550;
    
    pfilehdr->Machine = IMAGE_FILE_MACHINE_I386;
    pfilehdr->NumberOfSections = PE_TOTAL_SECTIONS;
    pfilehdr->TimeDateStamp = time(NULL);
    pfilehdr->SizeOfOptionalHeader = 0xE0;
    pfilehdr->Characteristics = IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LINE_NUMS_STRIPPED | IMAGE_FILE_LOCAL_SYMS_STRIPPED | IMAGE_FILE_32BIT_MACHINE;
    
    popthdr->Magic = IMAGE_NT_OPTIONAL_HDR_MAGIC;
    popthdr->MajorLinkerVersion = 6;
    popthdr->SizeOfCode = ROUNDUP(code.end_addr - code.base_addr, PE_SECTIONALIGN);
    popthdr->SizeOfInitializedData = ROUNDUP(data.end_addr - rodata.base_addr, PE_SECTIONALIGN);
    popthdr->AddressOfEntryPoint = ToRVA(GetSymbol("$ENTRY"));
    popthdr->BaseOfCode = PE_CODEBASE;
    popthdr->BaseOfData = PE_CODEBASE + popthdr->SizeOfCode;
    popthdr->ImageBase = PE_IMAGEBASE;
    popthdr->SectionAlignment = PE_SECTIONALIGN;
    popthdr->FileAlignment = PE_FILEALIGN;
    popthdr->MajorOperatingSystemVersion = 4;
    popthdr->MajorSubsystemVersion = 4;
    popthdr->SizeOfImage = ROUNDUP(data.end_addr - PE_IMAGEBASE, PE_SECTIONALIGN);
    popthdr->SizeOfHeaders = PE_CODEBASE;
    popthdr->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    popthdr->SizeOfStackReserve = 0x100000;
    popthdr->SizeOfStackCommit = 0x1000;
    popthdr->SizeOfHeapReserve = 0x100000;
    popthdr->SizeOfHeapCommit = 0x1000;
    popthdr->NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    

	ppedirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = ToRVA(GetSymbol("$IMPORT_DIRECTORY"));
	ppedirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = GetSymbol("$IMPORT_DIRECTORY.END") - GetSymbol("$IMPORT_DIRECTORY");
	
	ppedirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = ToRVA(GetSymbol("$IAT"));
	ppedirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = GetSymbol("$IAT.END") - GetSymbol("$IAT");

	fwrite(&nthdr, sizeof(nthdr), 1, fp);


	std::vector<DataBuffer *> sections{&code, &rodata, &data};
	const char *shdr_name[] = {".text", ".rdata", ".data"};
	DWORD shdr_flags[] = {0x60000020, 0x40000040, 0xC0000040};
	
	IMAGE_SECTION_HEADER shdr;
	for (size_t i = 0; i < sections.size(); i++) {
		auto seg = sections[i];
		memset(&shdr, 0, sizeof(shdr));
		
		strncpy((char *) shdr.Name, shdr_name[i], sizeof(shdr.Name));
		shdr.Misc.VirtualSize = seg->end_addr - seg->base_addr;
        shdr.VirtualAddress = ToRVA(seg->base_addr);
        shdr.SizeOfRawData = ROUNDUP(seg->end_addr - seg->base_addr, PE_SECTIONALIGN);
        shdr.PointerToRawData = ToRVA(seg->base_addr);
        shdr.Characteristics = shdr_flags[i];

		fwrite(&shdr, sizeof(shdr), 1, fp);
	}

	fseek(fp, popthdr->SizeOfImage - 1, SEEK_SET);
	fputc(0, fp);

	std::vector<uint8_t> segdata;
	for (auto &seg: sections) {
		segdata = seg->GetContent();
		fseek(fp, ToRVA(seg->base_addr), SEEK_SET);
		fwrite(segdata.data(), 1, segdata.size(), fp);
	}

	fclose(fp);
}
data_off_t CodeGen::ToRVA(data_off_t addr)
{
	return addr - PE_IMAGEBASE;
}

data_off_t CodeGen::GetSymbol(const std::string &sym)
{
	std::vector<DataBuffer *> sections{&code, &rodata, &data};
	for (auto &sect: sections) {
		data_off_t v = sect->GetSymbol(sym);
		if (v) return v;
	}
	return 0;
}
void CodeGen::Link()
{
	data_off_t base = PE_IMAGEBASE + PE_CODEBASE;


	std::vector<DataBuffer *> sections{&code, &rodata, &data};

	printf(" [*] Processing symbols ...\n");
	DataBuffer::ReduceSymbols(sections);

	printf(" [*] Calculating address ...\n");
	for (auto &sect: sections) {
		base = sect->CalcOffset(base);
		base = ROUNDUP(base, PE_SECTIONALIGN);
	}

	printf(" [*] Relocating ...\n");
	for (auto &sect: sections) {
		sect->DoRelocate();
	}

	printf(" [*] Making EXE ...\n");
	MakeEXE();
}

void CodeGen::DumpSections(const char *outfile)
{
	FILE *fp;
	if (outfile) fp = fopen(outfile, "w"); else fp = stdout;
	fprintf(fp, ".code:\n");
	code.Dump(fp);
	fprintf(fp, ".rodata:\n");
	rodata.Dump(fp);
	fprintf(fp, ".data:\n");
	data.Dump(fp);
	if (outfile) fclose(fp);
}
void CodeGen::GenerateCode()
{
	printf("[*] Generating type information ...\n");
	clsinfo = MiniJavaC::Instance()->goal->GetClassInfoList();
	//clsinfo.Dump();

	printf("[*] Generating code ...\n");

	printf(" [*] Generating code for main() ...\n");
	GenerateCodeForMainMethod(MiniJavaC::Instance()->goal->GetASTMainClass());

	for (auto &cls: clsinfo) {
		printf(" [*] Generating code for class %s ...\n", cls.GetName().c_str());
		for (auto &method: cls.method) {
			if (method.clsname == cls.GetName()) {
				printf("  [*] Generating code for %s::%s() ...\n", cls.GetName().c_str(), method.GetName().c_str());
				GenerateCodeForClassMethod(cls, method);
			}
		}
	}

	for (auto &cls: clsinfo) {
		printf(" [*] Generating virtual function table for class %s ...\n", cls.GetName().c_str());
		GenerateVtblForClass(cls);
	}

	printf("[*] Adding DLL import table ...\n");
	AddImportEntry("msvcrt", {"printf", "calloc", "exit"});
	MakeIAT();


	printf("[*] Linking ...\n");
	Link();
}
