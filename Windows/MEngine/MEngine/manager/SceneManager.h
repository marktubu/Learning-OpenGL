#pragma once

#include <string>
#include <unordered_map>

#include "../graphics/Scene.h"

class SceneManager {
public:
	static Scene* LoadScene(std::string name) {
		auto it = sceneMap.find(name);
		if (it != sceneMap.end()) {
			Scene::gScene = it->second;
			return it->second;
		}
		auto scene = new Scene(name);
		sceneMap[name] = scene;
		Scene::gScene = scene;
		return scene;
	}

	static std::unordered_map<std::string, Scene*> sceneMap;
};