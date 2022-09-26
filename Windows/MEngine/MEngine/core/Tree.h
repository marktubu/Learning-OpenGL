#pragma once

#include <list>
#include <functional>

class Node {
public:
    virtual ~Node() {}

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

    static void Post(Node* node, std::function<void(Node*)> func);

private:
    Node* root_node;


};