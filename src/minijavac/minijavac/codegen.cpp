#include "common.h"

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
	list.push_back(VarDeclListItem {
		VarDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
		},
		list.GetTotalSize(),
		node->GetASTType()->GetTypeSize(),
	});
}

data_off_t MethodDeclList::GetTotalSize()
{
	return this->size() * 4;
}

void MethodDeclList::Dump()
{
	for (auto &item: *this) {
		printf("  method: %08X %-10s %s():\n", (unsigned) item.off, ASTType::GetTypeName(item.decl.rettype.type), item.decl.name.c_str());
		item.decl.arg.Dump();
	}
}

void MethodDeclListVisitor::Visit(ASTMethodDeclaration *node, int level)
{
	list.push_back(MethodDeclListItem {
		MethodDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
			node->GetASTArgDeclarationList1()->GetVarDeclList(),
		},
		list.GetTotalSize(),
		std::dynamic_pointer_cast<ASTMethodDeclaration>(node->GetSharedPtr()),
	});
}

void ClassInfoItem::Dump()
{
	printf("class %s:\n", name.c_str());
	printf(" var:\n");
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
	list.push_back(ClassInfoItem{
		node->GetASTIdentifier()->id,
		node->GetASTVarDeclarationList()->GetVarDeclList(),
		node->GetASTMethodDeclarationList()->GetMethodDeclList(),
		std::dynamic_pointer_cast<ASTClassDeclaration>(node->GetSharedPtr()),
	});
}

