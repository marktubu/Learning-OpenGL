
#include <glad/glad.h>

#include "MeshRenderer.h"
#include "../core/GameObject.h"
#include "MeshFilter.h"
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"

void MeshRenderer::Render()
{
	auto filter = gameObject->GetComponent<MeshFilter>();
	auto mesh = filter->mesh;

	auto camera = Camera::Current;
	if ((camera->CullingMask & gameObject->layer) == 0x0) return;

	auto shader = material->shader;
	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 projection = camera->GetProjectionMatrix();
	shader->Use();
	shader->setMat4("model", model);
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);

	glBindVertexArray(mesh->VAO);

	auto textures = material->textures;
	for (auto i = 0;i < textures.size();i++) {
		auto texture = textures[i];
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		int num_diffuse = 1;
		int num_specular = 1;
		int num_height = 1;
		int num_normal = 1;
		std::string name;
		std::string type = texture.type;
		if (texture.type == "texture_diffuse") {
			name = type + std::to_string(num_diffuse++);
		}
		else if (texture.type == "texture_specular") {
			name = type + std::to_string(num_specular++);
		}
		else if (texture.type == "texture_height") {
			name = type + std::to_string(num_height++);
		}
		else if (texture.type == "texture_normal") {
			name = type + std::to_string(num_normal++);
		}
		shader->setInt("material." + name, i);
	}
	glActiveTexture(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
