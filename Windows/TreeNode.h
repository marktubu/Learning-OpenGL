
#include <list>

using namespace std;

class TreeNode {
public:
    void AddChild(TreeNode* node);
    void RemoveChild(TreeNode* node);

    TreeNode* Parent() { return parent; }
    void SetParent(TreeNode* p) { parent = p; }

    std::list<TreeNode*>& Children() { return children; }
private:
    std::list<TreeNode*> children;
    TreeNode* parent;
};

void TreeNode::AddChild(TreeNode* node) {
    if(node->Parent() != nullptr){
        node->Parent()->RemoveChild(node);
    }
    children.push_back(node);
    node->SetParent(this);
}

void TreeNode::RemoveChild(TreeNode* node) {
    children.remove(node);
}