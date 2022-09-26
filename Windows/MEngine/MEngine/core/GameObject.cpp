
#include "GameObject.h"
#include <rttr/registration>
using namespace rttr;

Tree GameObject::game_object_tree;

GameObject::GameObject(std::string _name): Node(), active(true), layer(0x01)
{
	name = _name;
	game_object_tree.Root()->AddChild(this);
}

GameObject::~GameObject()
{
}

void GameObject::AttachComponent(Component* comp) {
    comp->gameObject = this;
    type t = type::get(*comp);
    std::string comp_type_name = t.get_name().to_string();

    if (componentMap.find(comp_type_name) == componentMap.end()) {
        std::vector<Component*> comp_vec;
        comp_vec.push_back(comp);
        componentMap[comp_type_name] = comp_vec;
    }
    else {
        componentMap[comp_type_name].push_back(comp);
    }
}