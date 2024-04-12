#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

class Node {
public:
	Node(Node* parent)
		: m_Parent(parent)
		, m_Children()
		, Position(glm::vec3(0.f, 0.f, 0.f))
		, Rotation(glm::quat(1.f, 0.f, 0.f, 0.f))
		, Scale(1.f, 1.f, 1.f) {}

	glm::mat4x4 ComposeLocalMatrix() const;
	Node* GetParent() const;
	void AddChild(Node* node);

	glm::vec3 Position;
	glm::quat Rotation;
	glm::vec3 Scale;

private:
	Node* m_Parent = nullptr;
	std::vector<Node*> m_Children;
};

class Skeleton {
public:
	Skeleton() : m_RootNode(nullptr), m_Nodes() {}

	const std::vector<Node>& GetNodes() const;
	Node* GetRootNode() const;
	Node* AddNewNode(Node* parent);
	glm::mat4x4 ComposeGlobalMatrix(const Node* node) const;

private:
	Node* m_RootNode = nullptr;
	std::vector<Node> m_Nodes;
};