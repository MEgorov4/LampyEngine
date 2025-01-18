#pragma once
#include <vector>
#include <nlohmann/json.hpp>

class Node
{
protected:
	Node* m_parentNode;
	std::vector<Node*> m_childnodes;
	uint32_t uniqueID;

public:
	Node(Node* parentNode = nullptr);
	virtual ~Node();
	Node(const Node& n) = delete;
	const Node& operator=(const Node& n) = delete;

	void addNode(Node* node);
	void removeNode(Node* node);

	uint32_t getUniqueID();

protected:
	virtual void serialize(nlohmann::json& json);
	virtual void deserialize(const nlohmann::json& json);
};
