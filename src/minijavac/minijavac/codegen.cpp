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

void CodeGen::PopAndCheckType(std::shared_ptr<ASTNode> node, TypeInfo tinfo)
{
	PopAndCheckType(node.get(), tinfo);
}
void CodeGen::PopAndCheckType(ASTNode *node, TypeInfo tinfo)
{
	if (varstack.empty()) {
		MiniJavaC::Instance()->ReportError(node->loc, "internal error: stack empty");
		return;
	}
	TypeInfo &pinfo = varstack.back();
	
	if (tinfo != pinfo) {
		std::string msg = "type mismatch: " + pinfo.GetName() + ", expected " + tinfo.GetName();
		MiniJavaC::Instance()->ReportError(node->loc, msg.c_str());
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
	PopAndCheckType(node->GetASTExpression(), TypeInfo { ASTType::VT_INT });

	auto fmtstr = rodata.AppendItem(DataItem::New()->AddString("%d\n"));
	code.AppendItem(DataItem::New()->AddU8({0x68})->AddRel32(0, RelocInfo::RELOC_ABS, fmtstr)->SetComment("PUSH fmtstr"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.printf"))->SetComment("CALL printf"));
	code.AppendItem(DataItem::New()->AddU8({0x83, 0xC4, 0x08})->SetComment("ADD ESP,8"));
}
//virtual void Visit(ASTWhileStatement *node, int level);
//virtual void Visit(ASTIfElseStatement *node, int level);
//virtual void Visit(ASTBlockStatement *node, int level);

// expression
//virtual void Visit(ASTIdentifier *node, int level);
//virtual void Visit(ASTBoolean *node, int level);
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
	PopAndCheckType(node->GetRightASTExpression(), rtype);
	PopAndCheckType(node->GetLeftASTExpression(), ltype);

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
//virtual void Visit(ASTUnaryExpression *node, int level);
//virtual void Visit(ASTArrayLengthExpression *node, int level);
//virtual void Visit(ASTFunctionCallExpression *node, int level);
//virtual void Visit(ASTThisExpression *node, int level);
//virtual void Visit(ASTNewIntArrayExpression *node, int level);
//virtual void Visit(ASTNewExpression *node, int level);




void CodeGen::GenerateCodeForASTNode(std::shared_ptr<ASTNode> node)
{
	node->ASTNode::Accept(*this);
}
void CodeGen::GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls)
{
	code.ProvideSymbol("main");
	GenerateCodeForASTNode(maincls->GetASTStatement());
	code.AppendItem(DataItem::New()->AddU8({0x6A, 0x00})->SetComment("PUSH 0"));
	code.AppendItem(DataItem::New()->AddU8({0xE8})->AddRel32(0x5, RelocInfo::RELOC_REL, code.NewExternalSymbol("$MSVCRT.exit"))->SetComment("CALL exit"));
}
void CodeGen::GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method)
{
	code.ProvideSymbol(cls.GetName() + "." + method.GetName());

	code.AppendItem(DataItem::New()->AddU8({0x55})->SetComment("PUSH EBP"));
	code.AppendItem(DataItem::New()->AddU8({0x8B, 0xEC})->SetComment("MOV EBP,ESP"));
	
	data_off_t localsize = method.localvar.GetTotalSize();
	assert(localsize % 4 == 0);
	for (data_off_t i = 0; i < localsize / 4; i++) {
		code.AppendItem(DataItem::New()->AddU8({0x6A, 0x00})->SetComment("PUSH 0"));
	}


	GenerateCodeForASTNode(method.ptr->GetASTStatementList());

	// FIXME: return value
	
	code.AppendItem(DataItem::New()->AddU8({0xCC})->SetComment("== end of function"));
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
