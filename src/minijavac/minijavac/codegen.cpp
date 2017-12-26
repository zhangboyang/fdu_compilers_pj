#include "common.h"

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
	AddU32({val});
	reloc.push_back(RelocInfo {
		(data_off_t) bytes.size(),
		type,
		target,
	});
	return shared_from_this();
}


void DataBuffer::AppendItem(std::shared_ptr<DataItem> instr)
{
	list.push_back(instr);
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
		for (auto &r: item->reloc) {
			for (auto &p: extsym) { // FIXME: O(n^2)
				if (r.target == p.second) {
					printf("    reloc +%02X %s\n", r.off, p.first.c_str());
				}
			}
		}
		
	}
}


// Code Generator

CodeGen::CodeGen()
{
}
CodeGen *CodeGen::Instance()
{
	static CodeGen inst;
	return &inst;
}

void CodeGen::Visit(ASTStatement *node, int level)
{
	code.AppendItem(DataItem::New()->AddU8({0xCC})->SetComment("ERROR: unhandled statement"));
	printf("unhandled: %s\n", typeid(node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "internal error: unhandled statement");
	VisitChildren(node, level);
}
void CodeGen::Visit(ASTExpression *node, int level)
{
	code.AppendItem(DataItem::New()->AddU8({0xCC})->SetComment("ERROR: unhandled statement"));
	printf("unhandled: %s\n", typeid(node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "internal error: unhandled expression");
	VisitChildren(node, level);
}

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
	code.AppendItem(DataItem::New()->AddU8({0x81, 0xEC})->AddU32({(uint32_t) method.localvar.GetTotalSize()})->SetComment("SUB ESP,frame_size"));

	GenerateCodeForASTNode(method.ptr->GetASTStatementList());

	// FIXME: return value
	
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

	code.Dump();
}
