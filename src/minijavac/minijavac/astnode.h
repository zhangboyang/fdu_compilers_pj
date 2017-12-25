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
	int op;
public:
	ASTBinaryExpression(const yyltype &loc, std::initializer_list<ASTNode *> l, int op);
};

class ASTUnaryExpression : public ASTExpression {
	int op;
public:
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