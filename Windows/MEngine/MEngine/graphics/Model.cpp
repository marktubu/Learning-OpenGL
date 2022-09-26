
#include "Model.h"
#include "Texture.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "../core/GameObject.h"
#include "Shader.h"

void Model::Draw(Shader shader) {
	for (unsigned int i = 0;i < meshes.size();i++) {
		meshes[i].Draw(shader);
	}
}

void Model::loadModel(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR:ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	root = new GameObject(scene->mRootNode->mName.C_Str());
	processNode(scene->mRootNode, scene, root);
}

void Model::processNode(aiNode* node, const aiScene* scene, GameObject* go) {
	for (unsigned int i = 0;i < node->mNumMeshes;i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		MeshRenderer* renderer = go->AddComponent<MeshRenderer>();
		Mesh* m = processMesh(mesh, scene, renderer);
		auto filter = go->AddComponent<MeshFilter>();
		filter->mesh = m;
	}

	for (unsigned int i = 0;i < node->mNumChildren;i++) {
		auto child = node->mChildren[i];
		auto child_go = new GameObject(child->mName.C_Str());
		go->AddChild(child_go);
		processNode(node->mChildren[i], scene, child_go);
	}
}

Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene, MeshRenderer* renderer) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0;i < mesh->mNumVertices;i++) {
		Vertex vertex;
		aiVector3D mPosition = mesh->mVertices[i];
		glm::vec3 position;
		position.x = mPosition.x;
		position.y = mPosition.y;
		position.z = mPosition.z;
		vertex.Position = position;

		aiVector3D mNormal = mesh->mNormals[i];
		glm::vec3 normal;
		normal.x = mNormal.x;
		normal.y = mNormal.y;
		normal.z = mNormal.z;
		vertex.Normal = normal;

		if (mesh->HasTextureCoords(0)) {
			glm::vec2 texcoord;
			texcoord.x = mesh->mTextureCoords[0][i].x;
			texcoord.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = texcoord;
		}
		else {
			vertex.TexCoords = glm::vec2(0.f, 0.f);
		}

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0;i < mesh->mNumFaces;i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0;j < face.mNumIndices;j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* my_mesh = new Mesh(vertices, indices);
	Material* mat = new Material();
	renderer->material = mat;

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	mat->textures = textures;
	mat->shader = Shader::Find("geo");
	return my_mesh;
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	for (unsigned int i = 0;i < mat->GetTextureCount(type);i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0;j < textures_loaded.size();j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				skip = true;
				textures.push_back(textures_loaded[j]);
				break;
			}
		}
		if (!skip) {
			Texture texture;
			texture.Load(directory + '/' + str.C_Str());
			texture.SetType(TextureType(type));
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}

	return textures;
}
