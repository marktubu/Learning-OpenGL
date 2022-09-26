
#include "Tree.h"

Tree::Tree()
{
	root_node = new Node();
}

Tree::~Tree() = default;

void Tree::Post(Node* node, std::function<void(Node*)> func) {
    for (auto child : node->Children()) {
        Post(child, func);
    }
    //if (node != root_node) 
    {
        func(node);
    }
}