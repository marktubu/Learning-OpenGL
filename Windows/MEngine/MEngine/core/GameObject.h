#pragma once

#include <unordered_map>

#include "Tree.h"
#include "Component.h"

class GameObject : public Node {
public:
    GameObject(std::string name);
    ~GameObject();

    template<class T=Component>
    T AddComponent() {
        T* comp = new T();
        AttachComponent(comp);
        comp->Awake();
        return dynamic_cast<T*>(comp);
    }

    void AttachComponent(Component* comp);

public:
    std::string name;
    unsigned int layer;
    bool active;

private:
    unordered_map<string, std::vector<Component*>> componentMap;
    static Tree game_object_tree;
};
