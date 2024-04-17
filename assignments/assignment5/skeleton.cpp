#include "skeleton.h"

inline glm::mat4x4 Node::ComposeLocalMatrix() const {
    glm::mat4 m = glm::mat4(1.0f);
    m *= glm::mat4_cast(Rotation);
    m = glm::translate(m, Position);
    m = glm::scale(m, Scale);
    return m;
}

Node *Node::GetParent() const {
    return m_Parent;
}

void Node::AddChild(Node *node) {
    m_Children.push_back(node);
}

void Node::SetParent(Node *parent) {
    m_Parent = parent;
}

Node *Skeleton::GetNode(uint32_t index) {
    Node* node = &m_Nodes[index];
    if (node->valid) return node;
    return nullptr;
}

Node *Skeleton::GetRootNode() const {
    return m_RootNode;
}

Node *Skeleton::AddNewNode(Node *parent) {
    Node *newNode = &m_Nodes[m_NextNodeIndex++];
    newNode->SetParent(parent);
    newNode->valid = true;

    if (parent == nullptr) {
        m_RootNode = newNode;
    } else {
        parent->AddChild(newNode);
    }

    return newNode;
}

glm::mat4x4 Skeleton::ComposeGlobalMatrix(const Node *node) const {
    glm::mat4x4 matrix = glm::mat4x4(1.0f);

    const Node *head = node;
    while (head != nullptr) {
        matrix = head->ComposeLocalMatrix() * matrix;
        head = head->GetParent();
    }

    return matrix;
}
