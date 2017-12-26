#include "common.h"

const std::string &VarDeclItem::GetName() const
{
	return this->decl.name;
}

data_off_t VarDeclList::GetTotalSize()
{
	if (this->empty()) {
		return 0;
	} else {
		return this->back().off + this->back().size;
	}
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
	printf("unhandled: %s\n", typeid(node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "unhandled statement");
}
void CodeGen::Visit(ASTExpression *node, int level)
{
	printf("unhandled: %s\n", typeid(node).name());
	MiniJavaC::Instance()->ReportError(node->loc, "unhandled expression");
}

void CodeGen::GenerateCodeForASTNode(std::shared_ptr<ASTNode> node)
{
	node->ASTNode::Accept(*this);
}
void CodeGen::GenerateCodeForMainMethod(std::shared_ptr<ASTMainClass> maincls)
{
	// FIXME
	GenerateCodeForASTNode(maincls->GetASTStatement());
}
void CodeGen::GenerateCodeForClassMethod(ClassInfoItem &cls, MethodDeclItem &method)
{
	// FIXME
	GenerateCodeForASTNode(method.ptr->GetASTStatementList());
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
}
