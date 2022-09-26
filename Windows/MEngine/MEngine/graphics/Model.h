#pragma once
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"

class MeshRenderer;
class GameObject;
class Shader;

class Model {
public:
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	GameObject* root;

	Model(const std::string path) {
		loadModel(path);
	}

	void Draw(Shader shader);

private:
	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene, GameObject* go);
	Mesh* processMesh(aiMesh* mesh, const aiScene* scene, MeshRenderer* renderer);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

