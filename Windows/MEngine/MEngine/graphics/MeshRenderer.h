#pragma once

#include "../core/Component.h"

class Material;

class MeshRenderer : public Component {
public:


	Material* material;
	void Render();

RTTR_ENABLE();
};