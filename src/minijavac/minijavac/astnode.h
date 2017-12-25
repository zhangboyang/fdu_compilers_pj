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
	std::vector<std::shared_ptr<ASTNode> > ch;
public:
	ASTNode();
	ASTNode(std::initializer_list<ASTNode *> l);
	virtual ~ASTNode();
	std::shared_ptr<ASTNode> GetSharedPtr();
	void AddChild(std::shared_ptr<ASTNode> ch_ptr);
	void AddChild(ASTNode *ch_ptr);
	virtual void Visit(std::shared_ptr<ASTNodeVisitor> visitor);
};



//////////////// ASTNode derived classes ////////////////

class ASTIdentifier : public ASTNode {
	
};











//////////////// ASTNode Visitor ////////////////

class ASTNodeVisitor {
public:
	virtual void Visit(std::shared_ptr<ASTNode> node);
};
