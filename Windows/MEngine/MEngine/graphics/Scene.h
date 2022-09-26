#pragma once

#include <string>

#include "../core/GameObject.h"


class Scene {
public:
	Scene(std::string name = std::string()) {
		root = new GameObject(name);
	}

	static Scene* gScene;

	GameObject* root;
};