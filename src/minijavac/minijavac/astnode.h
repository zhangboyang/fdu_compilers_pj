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
private:
	virtual void Accept(ASTNodeVisitor &visitor, int level);
public:
	ASTNode();
	ASTNode(std::initializer_list<ASTNode *> l);
	virtual ~ASTNode();
	std::shared_ptr<ASTNode> GetSharedPtr();
	void AddChild(std::shared_ptr<ASTNode> ch_ptr);
	void AddChild(ASTNode *ch_ptr);
	void RecursiveAccept(ASTNodePreOrderVisitor &visitor, int level = 0);
	void RecursiveAccept(ASTNodePostOrderVisitor &visitor, int level = 0);
};



//////////////// ASTNode derived classes ////////////////

class ASTIdentifier : public ASTNode {
	std::string id;
private:
	//virtual void Accept(ASTNodeVisitor &visitor, int level) override;
public:
	ASTIdentifier(const std::string &id);
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