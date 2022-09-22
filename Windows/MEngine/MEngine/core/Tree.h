#pragma once

#include <list>

using namespace std;

class Node {
public:
    void AddChild(Node* node) {
        if (node->Parent() != nullptr) {
            node->Parent()->RemoveChild(node);
        }
        children.push_back(node);
        node->SetParent(this);
    }

    void RemoveChild(Node* node) {
        children.remove(node);
    }

    Node* Parent() { return parent; }
    void SetParent(Node* p) { parent = p; }

    std::list<Node*>& Children() { return children; }
private:
    std::list<Node*> children;
    Node* parent = nullptr;
};

class Tree {
public:
    Tree();
    ~Tree();

    Node* Root() { return root_node; }

private:
    Node* root_node;


};