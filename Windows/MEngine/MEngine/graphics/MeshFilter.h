#pragma once

#include <string>

#include "../core/Component.h"


class Mesh;
class MeshFilter : public Component {
public:

	void LoadMesh(std::string path) {

	}

	Mesh* mesh;

RTTR_ENABLE();
};