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
	printf("   %-8s %-8s %-10s %s\n", "offset", "size", "type", "name");
	for (auto &item: *this) {
		printf("   %08X %08X %-10s %s\n", (unsigned) item.off, (unsigned) item.size, ASTType::GetTypeName(item.decl.type.type), item.decl.name.c_str());
	}
	printf("   total size %08X\n", GetTotalSize());
}
void VarDeclListVisitor::Visit(ASTVarDeclaration *node, int level)
{
	var.push_back(VarDeclListItem {
		VarDecl {
			node->GetASTType()->GetTypeInfo(),
			node->GetASTIdentifier()->id,
		},
		var.GetTotalSize(),
		node->GetASTType()->GetTypeSize(),
	});
}


void ClassInfoVisitor::Visit(ASTClassDeclaration *node, int level)
{
	ClassInfo clsinfo;
	clsinfo.name = node->GetASTIdentifier()->id;
	
	


	node->GetASTVarDeclarationList()->GetVarDeclList().Dump();
}