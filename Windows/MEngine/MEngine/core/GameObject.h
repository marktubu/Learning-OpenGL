#pragma once

#include <unordered_map>

#include "Tree.h"
#include "Component.h"

class GameObject : public Node {
public:
    GameObject(std::string name);
    ~GameObject();


    template<class T=Component>
    T* AddComponent() {
        T* comp = new T();
        AttachComponent(comp);
        comp->Awake();
        return dynamic_cast<T*>(comp);
    }

    template<class T=Component>
    T* GetComponent() {
        type t = type::get<T>();
        std::string comp_type_name = t.get_name().to_string();
        auto iter = componentMap.find(comp_type_name);
        if (iter != componentMap.end()) {
            auto comps = iter->second;
            if (comps.size() > 0) {
                return dynamic_cast<T*>(comps[0]);
            }
        }
        return nullptr;
    }


    void AttachComponent(Component* comp);

public:
    std::string name;
    unsigned int layer;
    bool active;

private:
    std::unordered_map<std::string, std::vector<Component*>> componentMap;
    static Tree game_object_tree;
};
