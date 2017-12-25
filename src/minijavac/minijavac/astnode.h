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
class ASTNodePreOrderVisitor;
class ASTNodePostOrderVisitor;

class ASTNode {
	std::list<std::shared_ptr<ASTNode> >::iterator pool_handle;
	std::vector<std::shared_ptr<ASTNode> > ch;
public:
	yyltype loc;
private:
	virtual void Accept(ASTNodeVisitor &visitor, int level);
public:
	ASTNode();
	ASTNode(const yyltype &loc);
	ASTNode(const yyltype &loc, std::initializer_list<ASTNode *> l);
	virtual ~ASTNode();
	void Dump();
	std::shared_ptr<ASTNode> GetSharedPtr();
	void AddChild(std::shared_ptr<ASTNode> ch_ptr);
	void AddChild(ASTNode *ch_ptr);
	void RecursiveAccept(ASTNodePreOrderVisitor &visitor, int level = 0);
	void RecursiveAccept(ASTNodePostOrderVisitor &visitor, int level = 0);
};



//////////////// ASTNode derived classes ////////////////

// ASTExpresstion

class ASTExpression : public ASTNode {
	using ASTNode::ASTNode;
};


class ASTIdentifier : public ASTExpression {
	std::string id;
private:
	//virtual void Accept(ASTNodeVisitor &visitor, int level) override;
public:
	ASTIdentifier(const yyltype &loc, const std::string &id);
};

class ASTNumber : public ASTExpression {
	int val;
private:
	//virtual void Accept(ASTNodeVisitor &visitor, int level) override;
public:
	ASTNumber(const yyltype &loc, int val);
};

class ASTBoolean : public ASTExpression {
	int val;
private:
	//virtual void Accept(ASTNodeVisitor &visitor, int level) override;
public:
	ASTBoolean(const yyltype &loc, int val);
};




class ASTBinaryExpression : public ASTExpression {
public:
	int op;
	ASTBinaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op);
};

class ASTUnaryExpression : public ASTExpression {
public:
	int op;
	ASTUnaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op);
};

class ASTArrayLengthExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
};
class ASTFunctionCallExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
};
class ASTThisExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
};
class ASTNewIntArrayExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
};
class ASTNewExpression : public ASTExpression {
	using ASTExpression::ASTExpression;
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
};

class ASTStatementList : public ASTNode {
	using ASTNode::ASTNode;
};


class ASTArrayAssignStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};
class ASTAssignStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};
class ASTPrintlnStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};
class ASTWhileStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};
class ASTIfElseStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};
class ASTBlockStatement : public ASTStatement {
	using ASTStatement::ASTStatement;
};


// ASTType
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
};


// ASTArgDeclarationList

class ArgDeclarationList1 : public ASTNode {
	using ASTNode::ASTNode;
};
class ArgDeclarationList2 : public ASTNode {
	using ASTNode::ASTNode;
};



// ASTMethodDeclaration

class ASTMethodDeclaration : public ASTNode {
	using ASTNode::ASTNode;
};
class ASTMethodDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
};


// ASTVarDeclaration
class ASTVarDeclaration : public ASTNode {
	using ASTNode::ASTNode;
};
class ASTVarDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
};


// ASTClassDeclaration
class ASTClassDeclaration : public ASTNode {
	using ASTNode::ASTNode;
};
class ASTClassDeclarationList : public ASTNode {
	using ASTNode::ASTNode;
};


// ASTMainClass
class ASTMainClass : public ASTNode {
	using ASTNode::ASTNode;
};

// ASTGoal
class ASTGoal : public ASTNode {
	using ASTNode::ASTNode;
};


//////////////// ASTNode Visitor ////////////////

class ASTNodeVisitor {
public:
	virtual void Visit(ASTNode *node, int level);
};

class ASTNodePreOrderVisitor : public ASTNodeVisitor {
};
class ASTNodePostOrderVisitor : public ASTNodeVisitor {
};

class PrintVisitor : public ASTNodePreOrderVisitor {
	virtual void Visit(ASTNode *node, int level) override;
};