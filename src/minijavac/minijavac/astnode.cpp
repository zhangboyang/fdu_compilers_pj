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
}
ASTNode::ASTNode(std::initializer_list<ASTNode *> l) : ASTNode()
{
	for (auto ptr: l) {
		AddChild(ptr);
	}
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


void ASTNode::Accept(ASTNodeVisitor &visitor, int level)
{
	visitor.Visit(this, level);
}

void ASTNode::RecursiveAccept(ASTNodePreOrderVisitor &visitor, int level)
{
	Accept(visitor, level);
	for (auto &p: ch) {
		p->RecursiveAccept(visitor, level + 1);
	}
}

void ASTNode::RecursiveAccept(ASTNodePostOrderVisitor &visitor, int level)
{
	for (auto &p: ch) {
		p->RecursiveAccept(visitor, level + 1);
	}
	Accept(visitor, level);
}


//////////////// ASTNode derived classes ////////////////

ASTIdentifier::ASTIdentifier(const std::string &id) : id(id)
{
}


//////////////// default ASTNodeVisitor ////////////////

void ASTNodeVisitor::Visit(ASTNode *node, int level)
{
}