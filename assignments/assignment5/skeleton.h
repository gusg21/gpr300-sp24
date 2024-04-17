#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <array>

#define SKELETON_MAX_SIZE 128

class Node {
public:
	explicit Node()
		: m_Parent(nullptr)
		, m_Children()
		, Position(glm::vec3(0.f, 0.f, 0.f))
		, Rotation(glm::quat(1.f, 0.f, 0.f, 0.f))
		, Scale(1.f, 1.f, 1.f) {}

	glm::mat4x4 ComposeLocalMatrix() const;
	Node* GetParent() const;
    void SetParent(Node* parent);
	void AddChild(Node* node);

	glm::vec3 Position;
	glm::quat Rotation;
	glm::vec3 Scale;

    bool valid = false;

private:
	Node* m_Parent = nullptr;
	std::vector<Node*> m_Children;
};

class Skeleton {
public:
    Skeleton() = default;

    Node* GetNode(uint32_t index);
	Node* GetRootNode() const;
	Node* AddNewNode(Node* parent);
	glm::mat4x4 ComposeGlobalMatrix(const Node* node) const;

private:
	Node* m_RootNode = nullptr;
	std::array<Node, SKELETON_MAX_SIZE> m_Nodes;
    uint32_t m_NextNodeIndex = 0;
};