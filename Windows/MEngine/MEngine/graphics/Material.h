#pragma once

#include <vector>

#include "Texture.h"

class Shader;

class Material {
public:
	Shader* shader;
	std::vector<Texture> textures;
};