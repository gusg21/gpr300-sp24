#include "skeleton.h"

inline glm::mat4x4 Node::ComposeLocalMatrix() const {
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, Position);
	m *= glm::mat4_cast(Rotation);
	m = glm::scale(m, Scale);
	return m;
}

Node* Node::GetParent() const
{
	return m_Parent;
}

void Node::AddChild(Node* node)
{
	m_Children.push_back(node);
}

const std::vector<Node>& Skeleton::GetNodes() const
{
	return m_Nodes;
}

Node* Skeleton::GetRootNode() const
{
	return m_RootNode;
}

Skeleton::AddNewNode(Node parent, Node node)
{
	if (parent == nullptr)
	{
		m_RootNode = newNode;
	}
	else 
	{
		parent->AddChild(newNode);
	}

	return newNode;
}

glm::mat4x4 Skeleton::ComposeGlobalMatrix(const Node* node) const
{
	glm::mat4x4 matrix = glm::mat4x4(1.0f);

	const Node* head = node;
	while (head != m_RootNode) {
		matrix *= head->ComposeLocalMatrix();
		head = head->GetParent();
	}

	return matrix;
}
