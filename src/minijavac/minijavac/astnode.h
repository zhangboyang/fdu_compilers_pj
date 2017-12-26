#pragma once

//////////////// ASTNodePool ////////////////

class ASTNode;

class ASTNodePool {
	friend ASTNode;
	
	std::list<std::shared_ptr<ASTNode> > pool;
private:
	ASTNodePool();
	std::list<std::shared_ptr<ASTNode> >::iterator RegisterNode(ASTNode *ptr);
	std::shared_ptr<ASTNode> GetSharedPtr(std::list<std::shared_ptr<ASTNode> >::iterator handle);
public:
	static ASTNodePool *Instance();
	void Shrink();
};


//////////////// ASTNode ////////////////

class ASTNodeVisitor;

class ASTNode {
	std::list<std::shared_ptr<ASTNode> >::iterator pool_handle;
public:
	std::vector<std::shared_ptr<ASTNode> > ch;
	yyltype loc;
private:
	
public:
	ASTNode();
	ASTNode(const yyltype &loc);
	ASTNode(const yyltype &loc, std::initializer_list<ASTNode *> l);
	virtual ~ASTNode();
	void Dump();
	void DumpTree();
	virtual void Accept(ASTNodeVisitor &visitor, int level);
	void Accept(ASTNodeVisitor &visitor);
	std::shared_ptr<ASTNode> GetSharedPtr();
	void AddChild(std::shared_ptr<ASTNode> ch_ptr);
	void AddChild(ASTNode *ch_ptr);
};



//////////////// ASTNode derived classes ////////////////

// ASTExpresstion

class ASTExpression : public ASTNode {
	using ASTNode::ASTNode;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};


class ASTIdentifier : public ASTExpression {
public:
	std::string id;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	ASTIdentifier(const yyltype &loc, const std::string &id);
};

class ASTNumber : public ASTExpression {
public:
	int val;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	ASTNumber(const yyltype &loc, int val);
};

class ASTBoolean : public ASTExpression {
public:
	int val;
private:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
public:
	ASTBoolean(const yyltype &loc, int val);
};




class ASTBinaryExpression : public ASTExpression {
public:
	int op;
	ASTBinaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op);
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	const char *GetOperatorName();
};

class ASTUnaryExpression : public ASTExpression {
public:
	int op;
	ASTUnaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op);
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	const char *GetOperatorName();
};

class ASTArrayLengthExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTFunctionCallExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTThisExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTNewIntArrayExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTNewExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};

class ASTArgExpressionList1 : public ASTNode {
	using ASTNode::ASTNode;
};
class ASTArgExpressionList2 : public ASTNode {
	using ASTNode::ASTNode;
};


// ASTStatment

class ASTStatement : public ASTNode {
	using ASTNode::ASTNode;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};

class ASTStatementList : public ASTNode {
	using ASTNode::ASTNode;
};


class ASTArrayAssignStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTAssignStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTPrintlnStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTWhileStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTIfElseStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};
class ASTBlockStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
};


// ASTType

class TypeInfo;

class ASTType : public ASTNode {
public:
	enum VarType {
		VT_INT,
		VT_INTARRAY,
		VT_BOOLEAN,
		VT_CLASS,
	};

	VarType type;
	ASTType(const yyltype &loc, std::initializer_list<ASTNode *> l, VarType type);
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	const char *GetTypeName();
	static const char *GetTypeName(VarType type);
	TypeInfo GetTypeInfo();
	std::shared_ptr<ASTIdentifier> GetASTIdentifier();
	data_off_t GetTypeSize();
};


// ASTArgDeclarationList
class VarDeclList;
class ASTArgDeclarationList1 : public ASTNode {
	using ASTNode::ASTNode;
public:
	VarDeclList GetVarDeclList();
};
class ASTArgDeclarationList2 : public ASTNode {
	using ASTNode::ASTNode;
};



// ASTMethodDeclaration
class MethodDeclList;
class ASTVarDeclarationList;
class ASTMethodDeclaration : public ASTNode {
	using ASTNode::ASTNode;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	std::shared_ptr<ASTType> GetASTType();
	std::shared_ptr<ASTIdentifier> GetASTIdentifier();
	std::shared_ptr<ASTArgDeclarationList1> GetASTArgDeclarationList1();
	std::shared_ptr<ASTVarDeclarationList> GetASTVarDeclarationList();
	std::shared_ptr<ASTStatementList> GetASTStatementList();
};
class ASTMethodDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
public:
	MethodDeclList GetMethodDeclList();
};


// ASTVarDeclaration
class VarDeclList;
class ASTVarDeclaration : public ASTNode {
	using ASTNode::ASTNode;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	std::shared_ptr<ASTType> GetASTType();
	std::shared_ptr<ASTIdentifier> GetASTIdentifier();
};

class ASTVarDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
public:
	VarDeclList GetVarDeclList();
};


// ASTClassDeclaration
class ASTClassDeclaration : public ASTNode {
	using ASTNode::ASTNode;
public:
	virtual void Accept(ASTNodeVisitor &visitor, int level) override;
	std::shared_ptr<ASTIdentifier> GetASTIdentifier();
	std::shared_ptr<ASTVarDeclarationList> GetASTVarDeclarationList();
	std::shared_ptr<ASTMethodDeclarationList> GetASTMethodDeclarationList();
};
class ASTClassDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
};


// ASTMainClass
class ASTMainClass : public ASTNode {
	using ASTNode::ASTNode;
public:
	std::shared_ptr<ASTStatement> GetASTStatement();
};

// ASTGoal
class ClassInfoList;
class ASTGoal : public ASTNode {
	using ASTNode::ASTNode;
public:
	ClassInfoList GetClassInfoList();
	std::shared_ptr<ASTMainClass> GetASTMainClass();
};


//////////////// ASTNode Visitor ////////////////

class ASTNodeVisitor {
protected:
	void VisitChildren(ASTNode *node, int level);
public:
	virtual void Visit(ASTNode *node, int level);

	virtual void Visit(ASTType *node, int level);

	virtual void Visit(ASTClassDeclaration *node, int level);
	virtual void Visit(ASTVarDeclaration *node, int level);
	virtual void Visit(ASTMethodDeclaration *node, int level);

	// statment
	virtual void Visit(ASTStatement *node, int level);
	virtual void Visit(ASTArrayAssignStatement *node, int level);
	virtual void Visit(ASTAssignStatement *node, int level);
	virtual void Visit(ASTPrintlnStatement *node, int level);
	virtual void Visit(ASTWhileStatement *node, int level);
	virtual void Visit(ASTIfElseStatement *node, int level);
	virtual void Visit(ASTBlockStatement *node, int level);

	// expression
	virtual void Visit(ASTExpression *node, int level);
	virtual void Visit(ASTIdentifier *node, int level);
	virtual void Visit(ASTBoolean *node, int level);
	virtual void Visit(ASTNumber *node, int level);
	virtual void Visit(ASTBinaryExpression *node, int level);
	virtual void Visit(ASTUnaryExpression *node, int level);
	virtual void Visit(ASTArrayLengthExpression *node, int level);
	virtual void Visit(ASTFunctionCallExpression *node, int level);
	virtual void Visit(ASTThisExpression *node, int level);
	virtual void Visit(ASTNewIntArrayExpression *node, int level);
	virtual void Visit(ASTNewExpression *node, int level);
};


class PrintVisitor : public ASTNodeVisitor {
	FILE *fp;
	bool dumpcontent;

	virtual void Visit(ASTNode *node, int level) override;
	virtual void Visit(ASTIdentifier *node, int level) override;
	virtual void Visit(ASTBoolean *node, int level) override;
	virtual void Visit(ASTNumber *node, int level) override;
	virtual void Visit(ASTBinaryExpression *node, int level) override;
	virtual void Visit(ASTUnaryExpression *node, int level) override;
	virtual void Visit(ASTType *node, int level) override;
	void Visit(ASTNode *node, int level, std::function<void()> func);

public:
	void DumpASTToTextFile(const char *txtfile, ASTNode *root, bool dumpcontent);
	void DumpTree(ASTNode *root, bool dumpcontent);
};

class JSONVisitor : public ASTNodeVisitor {
	FILE *fp;

	void OutEscapedString(const char *s);
	void OutQuotedString(const char *s);

	void OutKeyValue(const char *key, const char *value);
	void OutKeyValue(const char *key, int value);

	virtual void Visit(ASTNode *node, int level) override;
	virtual void Visit(ASTIdentifier *node, int level) override;
	virtual void Visit(ASTBoolean *node, int level) override;
	virtual void Visit(ASTNumber *node, int level) override;
	virtual void Visit(ASTBinaryExpression *node, int level) override;
	virtual void Visit(ASTUnaryExpression *node, int level) override;
	virtual void Visit(ASTType *node, int level) override;
	void Visit(ASTNode *node, int level, std::function<void()> func);

public:
	void DumpASTToJSON(const char *jsonfile, ASTNode *root, const std::vector<std::string> &lines);
};