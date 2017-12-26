#include "common.h"
#include "minijavac.tab.h"

//////////////// ASTNodePool ////////////////

ASTNodePool::ASTNodePool()
{
}
ASTNodePool *ASTNodePool::Instance()
{
	static ASTNodePool inst;
	return &inst;
}
std::list<std::shared_ptr<ASTNode> >::iterator ASTNodePool::RegisterNode(ASTNode *ptr)
{
	pool.push_front(std::shared_ptr<ASTNode>(ptr));
	return pool.begin();
}
std::shared_ptr<ASTNode> ASTNodePool::GetSharedPtr(std::list<std::shared_ptr<ASTNode> >::iterator handle)
{
	return *handle;
}
void ASTNodePool::Shrink()
{
	bool flag;
	do {
		flag = false;
		for (auto it = pool.begin(); it != pool.end(); ) {
			if (it->use_count() == 1) {
				printf("ASTNodePool::Shrink() delete %p\n", it->get());
				pool.erase(it++);
				flag = true;
			} else {
				it++;
			}
		}
	} while (flag);
}






//////////////// ASTNode ////////////////

ASTNode::ASTNode()
{
	pool_handle = ASTNodePool::Instance()->RegisterNode(this);
	memset(&loc, 0, sizeof(loc));
}
ASTNode::ASTNode(const yyltype &loc) : ASTNode()
{
	this->loc = loc;
}
ASTNode::ASTNode(const yyltype &loc, std::initializer_list<ASTNode *> l) : ASTNode(loc)
{
	for (auto ptr: l) {
		AddChild(ptr);
	}
}

ASTNode::~ASTNode()
{
}

void ASTNode::Dump()
{
	printf("Dump %p: %s\n", this, typeid(*this).name());
	MiniJavaC::Instance()->DumpContent(loc);
}

std::shared_ptr<ASTNode> ASTNode::GetSharedPtr()
{
	return ASTNodePool::Instance()->GetSharedPtr(pool_handle);
}

void ASTNode::AddChild(std::shared_ptr<ASTNode> ch_ptr)
{
	ch.push_back(ch_ptr);
}

void ASTNode::AddChild(ASTNode *ch_ptr)
{
	AddChild(ch_ptr->GetSharedPtr());
}


void ASTNode::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}


//////////////// ASTNode derived classes ////////////////

ASTIdentifier::ASTIdentifier(const yyltype &loc, const std::string &id) : ASTExpression(loc), id(id)
{
}
ASTNumber::ASTNumber(const yyltype &loc, int val) : ASTExpression(loc), val(val)
{
}
ASTBoolean::ASTBoolean(const yyltype &loc, int val) : ASTExpression(loc), val(val)
{
}
ASTBinaryExpression::ASTBinaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op) : ASTExpression(loc, l), op(op)
{
}
const char *ASTBinaryExpression::GetOperatorName()
{
	switch (op) {
		case TOK_LAND:  return "LOGICAL_AND";
		case TOK_LT:    return "LESS_THAN";
		case TOK_ADD:   return "ADD";
		case TOK_SUB:   return "SUB";
		case TOK_MUL:   return "MUL";
		case TOK_LS:    return "ARRAY_SUBSCRIPT";
		default:        return "UNKNOWN";
	}
}
ASTUnaryExpression::ASTUnaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op) : ASTExpression(loc, l), op(op)
{
}
const char *ASTUnaryExpression::GetOperatorName()
{
	switch (op) {
		case TOK_NOT:   return "NOT";
		case TOK_LP:    return "PARENTHESES";
		default:        return "UNKNOWN";
	}
}



ASTType::ASTType(const yyltype &loc, std::initializer_list<ASTNode *> l, VarType type) : ASTNode(loc, l), type(type)
{
}
const char *ASTType::GetTypeName()
{
	switch (type) {
		case VT_INT:       return "INT";
		case VT_INTARRAY:  return "INT_ARRAY";
		case VT_BOOLEAN:   return "BOOLEAN";
		case VT_CLASS:     return "CLASS";
		default:           return "UNKNOWN";
	}
}


//////////////// default ASTNodeVisitor ////////////////


void ASTNodeVisitor::VisitChildren(ASTNode *node, int level)
{
	for (auto &ch: node->ch) {
		ch->Accept(*this, level + 1);
	}
}

// Visit
void ASTNodeVisitor::Visit(ASTNode *node, int level)
{
	VisitChildren(node, level);
}
void ASTNodeVisitor::Visit(ASTIdentifier *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTBoolean *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTNumber *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTBinaryExpression *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTUnaryExpression *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTType *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}
void ASTNodeVisitor::Visit(ASTClassDeclaration *node, int level)
{
	Visit(static_cast<ASTNode *>(node), level);
}

// Accept
void ASTIdentifier::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}
void ASTNumber::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}
void ASTBoolean::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}
void ASTBinaryExpression::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}
void ASTUnaryExpression::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}
void ASTType::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}

void ASTClassDeclaration::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}