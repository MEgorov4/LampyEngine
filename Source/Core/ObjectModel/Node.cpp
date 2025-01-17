#include "Node.h"

Node::Node(Node* parentNode) : m_parentNode(parentNode)
{
    static uint32_t ID = 0;
    uniqueID = ID++;
}

Node::~Node()
{
	for (Node* node : m_childnodes)
	{
		delete node;
        node = nullptr;
	}
}

void Node::addNode(Node* node)
{
    node->m_parentNode = this;
    m_childnodes.push_back(node);
}

void Node::removeNode(Node* node)
{
    auto it = std::find(m_childnodes.begin(), m_childnodes.end(), node);
    if (it != m_childnodes.end()) {
        m_childnodes.erase(it);
        node->m_parentNode = nullptr;  
    }
}

uint32_t Node::getUniqueID()
{
    return uniqueID;
}

void Node::render(VkCommandBuffer commandBuffer)
{
    for (auto node : m_childnodes)
    {
        assert(node);
        node->render(commandBuffer);
    }
}

void Node::serialize(nlohmann::json& json)
{
    json["uniqueID"] = uniqueID;

    json["children"] = nlohmann::json::array(); 

    for (Node* child : m_childnodes) {
        nlohmann::json childJson;
        child->serialize(childJson);  
        json["children"].push_back(childJson); 
    }
}

void Node::deserialize(const nlohmann::json& json)
{
    uniqueID = json["uniqueID"];

    if (json.contains("children")) {
        for (const auto& childJson : json["children"]) {
            Node* childNode = new Node(this); 
            childNode->deserialize(childJson);  
            addNode(childNode); 
        }
    }
}
