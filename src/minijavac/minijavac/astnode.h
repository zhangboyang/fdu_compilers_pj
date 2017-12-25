#pragma once


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



class ASTNodeVisitor;

class ASTNode {
	std::list<std::shared_ptr<ASTNode> >::iterator pool_handle;
	std::vector<std::shared_ptr<ASTNode> > ch;
public:
	ASTNode();
	virtual ~ASTNode();
	std::shared_ptr<ASTNode> GetSharedPtr();
	void AddChild(std::shared_ptr<ASTNode> ch_ptr);
	void AddChild(ASTNode *ch_ptr);
	void Visit(std::shared_ptr<ASTNodeVisitor> visitor);
};





class ASTNodeVisitor {
public:
	virtual void Visit(std::shared_ptr<ASTNode> node);
};
