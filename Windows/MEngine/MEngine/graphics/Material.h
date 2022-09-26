#pragma once

#include <vector>

#include "Shader.h"
#include "Texture.h"

class Material {
public:
	Shader* shader;
	std::vector<Texture> textures;
};