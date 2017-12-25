#include "common.h"

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
	for (auto it = pool.begin(); it != pool.end(); ) {
		if (it->use_count() == 1) {
			printf("ASTNodePool free %p\n", it->get());
			pool.erase(it++);
		} else {
			it++;
		}
	}
}






//////////////// ASTNode ////////////////

ASTNode::ASTNode()
{
	pool_handle = ASTNodePool::Instance()->RegisterNode(this);
}

ASTNode::~ASTNode()
{
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

void ASTNode::Visit(std::shared_ptr<ASTNodeVisitor> visitor)
{
	visitor->Visit(GetSharedPtr());
}
