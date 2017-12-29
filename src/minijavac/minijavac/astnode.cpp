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
				//printf("ASTNodePool::Shrink() delete %p\n", it->get());
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

void ASTNode::DumpTree()
{
	PrintVisitor v;
	printf("DumpTree %p: %s\n", this, typeid(*this).name());
	v.DumpTree(this, true);
}

void ASTNode::Accept(ASTNodeVisitor &visitor)
{
	Accept(visitor, 0);
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


std::shared_ptr<ASTStatement> ASTMainClass::GetASTStatement()
{
	return std::dynamic_pointer_cast<ASTStatement>(ch[2]);
}
std::shared_ptr<ASTMainClass> ASTGoal::GetASTMainClass()
{
	return std::dynamic_pointer_cast<ASTMainClass>(ch[0]);
}
ClassInfoList ASTGoal::GetClassInfoList()
{
	ClassInfoVisitor v;
	ASTNode::Accept(v);
	return std::move(v.list);
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






////// Expression
std::shared_ptr<ASTExpression> ASTBinaryExpression::GetLeftASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTBinaryExpression::GetRightASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[1]);
}
std::shared_ptr<ASTExpression> ASTPrintlnStatement::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTUnaryExpression::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTArrayLengthExpression::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTNewIntArrayExpression::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}


std::shared_ptr<ASTExpression> ASTFunctionCallExpression::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTIdentifier> ASTFunctionCallExpression::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[1]);
}
std::shared_ptr<ASTArgExpressionList1> ASTFunctionCallExpression::GetASTArgExpressionList1()
{
	return std::dynamic_pointer_cast<ASTArgExpressionList1>(ch[2]);
}


////// Statement
std::shared_ptr<ASTIdentifier> ASTArrayAssignStatement::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTArrayAssignStatement::GetSubscriptASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[1]);
}
std::shared_ptr<ASTExpression> ASTArrayAssignStatement::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[2]);
}
std::shared_ptr<ASTIdentifier> ASTAssignStatement::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}
std::shared_ptr<ASTExpression> ASTAssignStatement::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[1]);
}
std::shared_ptr<ASTExpression> ASTWhileStatement::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTStatement> ASTWhileStatement::GetASTStatement()
{
	return std::dynamic_pointer_cast<ASTStatement>(ch[1]);
}
std::shared_ptr<ASTExpression> ASTIfElseStatement::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[0]);
}
std::shared_ptr<ASTStatement> ASTIfElseStatement::GetThenASTStatement()
{
	return std::dynamic_pointer_cast<ASTStatement>(ch[1]);
}
std::shared_ptr<ASTStatement> ASTIfElseStatement::GetElseASTStatement()
{
	return std::dynamic_pointer_cast<ASTStatement>(ch[2]);
}





ASTType::ASTType(const yyltype &loc, std::initializer_list<ASTNode *> l, VarType type) : ASTNode(loc, l), type(type)
{
}
const char *ASTType::GetTypeName()
{
	return GetTypeName(type);
}
const char *ASTType::GetTypeName(ASTType::VarType type)
{
	switch (type) {
		case VT_INT:       return "INT";
		case VT_INTARRAY:  return "INT_ARRAY";
		case VT_BOOLEAN:   return "BOOLEAN";
		case VT_CLASS:     return "CLASS";
		default:           return "UNKNOWN";
	}
}
std::shared_ptr<ASTIdentifier> ASTType::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}
TypeInfo ASTType::GetTypeInfo()
{
	if (type != VT_CLASS) {
		return TypeInfo { type };
	} else {
		return TypeInfo { type, GetASTIdentifier()->id };
	}
}
data_off_t ASTType::GetTypeSize()
{
	switch (type) {
		case VT_INT:       return 4;
		case VT_INTARRAY:  return 8;
		case VT_BOOLEAN:   return 4;
		case VT_CLASS:     return 4;
		default: panic();
	}
}


std::shared_ptr<ASTIdentifier> ASTClassDeclaration::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}
std::shared_ptr<ASTVarDeclarationList> ASTClassDeclaration::GetASTVarDeclarationList()
{
	return std::dynamic_pointer_cast<ASTVarDeclarationList>(ch[1]);
}
std::shared_ptr<ASTMethodDeclarationList> ASTClassDeclaration::GetASTMethodDeclarationList()
{
	return std::dynamic_pointer_cast<ASTMethodDeclarationList>(ch[2]);
}

std::shared_ptr<ASTIdentifier> ASTDerivedClassDeclaration::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}
std::shared_ptr<ASTIdentifier> ASTDerivedClassDeclaration::GetBaseASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[1]);
}
std::shared_ptr<ASTVarDeclarationList> ASTDerivedClassDeclaration::GetASTVarDeclarationList()
{
	return std::dynamic_pointer_cast<ASTVarDeclarationList>(ch[2]);
}
std::shared_ptr<ASTMethodDeclarationList> ASTDerivedClassDeclaration::GetASTMethodDeclarationList()
{
	return std::dynamic_pointer_cast<ASTMethodDeclarationList>(ch[3]);
}


std::shared_ptr<ASTType> ASTVarDeclaration::GetASTType()
{
	return std::dynamic_pointer_cast<ASTType>(ch[0]);
}
std::shared_ptr<ASTIdentifier> ASTVarDeclaration::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[1]);
}

std::shared_ptr<ASTType> ASTMethodDeclaration::GetASTType()
{
	return std::dynamic_pointer_cast<ASTType>(ch[0]);
}
std::shared_ptr<ASTIdentifier> ASTMethodDeclaration::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[1]);
}
std::shared_ptr<ASTArgDeclarationList1> ASTMethodDeclaration::GetASTArgDeclarationList1()
{
	return std::dynamic_pointer_cast<ASTArgDeclarationList1>(ch[2]);
}
std::shared_ptr<ASTVarDeclarationList> ASTMethodDeclaration::GetASTVarDeclarationList()
{
	return std::dynamic_pointer_cast<ASTVarDeclarationList>(ch[3]);
}
std::shared_ptr<ASTStatementList> ASTMethodDeclaration::GetASTStatementList()
{
	return std::dynamic_pointer_cast<ASTStatementList>(ch[4]);
}
std::shared_ptr<ASTExpression> ASTMethodDeclaration::GetASTExpression()
{
	return std::dynamic_pointer_cast<ASTExpression>(ch[5]);
}
std::shared_ptr<ASTIdentifier> ASTNewExpression::GetASTIdentifier()
{
	return std::dynamic_pointer_cast<ASTIdentifier>(ch[0]);
}


MethodDeclList ASTMethodDeclarationList::GetMethodDeclList(MethodDeclList base, const std::string &clsname)
{
	MethodDeclListVisitor v(base, clsname);
	ASTNode::Accept(v);
	return std::move(v.list);
}

VarDeclList ASTArgDeclarationList1::GetVarDeclList(VarDeclList base)
{
	VarDeclListVisitor v(base);
	ASTNode::Accept(v);
	return std::move(v.list);
}
VarDeclList ASTVarDeclarationList::GetVarDeclList(VarDeclList base)
{
	VarDeclListVisitor v(base);
	ASTNode::Accept(v);
	return std::move(v.list);
}

//////////////// default ASTNodeVisitor ////////////////


void ASTNodeVisitor::VisitChildren(ASTNode *node, int level)
{
	for (auto &ch: node->ch) {
		ch->Accept(*this, level + 1);
	}
}

////////// Visit
void ASTNodeVisitor::Visit(ASTNode *node, int level)
{
	VisitChildren(node, level);
}

#define MAKE_VISIT(super, cls) \
	void ASTNodeVisitor::Visit(cls *node, int level) \
	{ \
		Visit(static_cast<super *>(node), level); \
	}

MAKE_VISIT(ASTNode, ASTType)

MAKE_VISIT(ASTNode, ASTClassDeclaration)
MAKE_VISIT(ASTNode, ASTDerivedClassDeclaration)
MAKE_VISIT(ASTNode, ASTVarDeclaration)
MAKE_VISIT(ASTNode, ASTMethodDeclaration)

// statment
MAKE_VISIT(ASTNode, ASTStatement)
MAKE_VISIT(ASTStatement, ASTArrayAssignStatement)
MAKE_VISIT(ASTStatement, ASTAssignStatement)
MAKE_VISIT(ASTStatement, ASTPrintlnStatement)
MAKE_VISIT(ASTStatement, ASTWhileStatement)
MAKE_VISIT(ASTStatement, ASTIfElseStatement)
MAKE_VISIT(ASTStatement, ASTBlockStatement)

// expression
MAKE_VISIT(ASTNode, ASTExpression)
MAKE_VISIT(ASTExpression, ASTIdentifier)
MAKE_VISIT(ASTExpression, ASTBoolean)
MAKE_VISIT(ASTExpression, ASTNumber)
MAKE_VISIT(ASTExpression, ASTBinaryExpression)
MAKE_VISIT(ASTExpression, ASTUnaryExpression)
MAKE_VISIT(ASTExpression, ASTArrayLengthExpression)
MAKE_VISIT(ASTExpression, ASTFunctionCallExpression)
MAKE_VISIT(ASTExpression, ASTThisExpression)
MAKE_VISIT(ASTExpression, ASTNewIntArrayExpression)
MAKE_VISIT(ASTExpression, ASTNewExpression)


/////////// Accept
#define MAKE_ACCEPT(cls) \
	void cls::Accept(ASTNodeVisitor &visitor, int level) \
	{ \
		visitor.Visit(this, level); \
	}

MAKE_ACCEPT(ASTNode)

MAKE_ACCEPT(ASTType)

MAKE_ACCEPT(ASTClassDeclaration)
MAKE_ACCEPT(ASTDerivedClassDeclaration)
MAKE_ACCEPT(ASTVarDeclaration)
MAKE_ACCEPT(ASTMethodDeclaration)

// statment
MAKE_ACCEPT(ASTStatement)
MAKE_ACCEPT(ASTArrayAssignStatement)
MAKE_ACCEPT(ASTAssignStatement)
MAKE_ACCEPT(ASTPrintlnStatement)
MAKE_ACCEPT(ASTWhileStatement)
MAKE_ACCEPT(ASTIfElseStatement)
MAKE_ACCEPT(ASTBlockStatement)

// expression
MAKE_ACCEPT(ASTExpression)
MAKE_ACCEPT(ASTIdentifier)
MAKE_ACCEPT(ASTBoolean)
MAKE_ACCEPT(ASTNumber)
MAKE_ACCEPT(ASTBinaryExpression)
MAKE_ACCEPT(ASTUnaryExpression)
MAKE_ACCEPT(ASTArrayLengthExpression)
MAKE_ACCEPT(ASTFunctionCallExpression)
MAKE_ACCEPT(ASTThisExpression)
MAKE_ACCEPT(ASTNewIntArrayExpression)
MAKE_ACCEPT(ASTNewExpression)
