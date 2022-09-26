#pragma once

#include <string>

#include <assimp/types.h>
#include <glad/glad.h>


enum TextureType {
	None,
	Diffuse,
	Specular,
	Ambient,
	Emissive,
	Height,
	Normal,
};

class Texture {
public:
	static Texture* Create(std::string path);

	static void Fill(Texture* tex, std::string path);

	void Load(std::string path);

	void SetType(TextureType _type) {
		switch (_type) {
		case TextureType::Diffuse:
			type = "texture_diffuse";
			break;
		case TextureType::Specular:
			type = "texture_specular";
			break;
		case TextureType::Ambient:
			type = "texture_ambient";
			break;
		case TextureType::Height:
			type = "texture_height";
			break;
		case TextureType::Normal:
			type = "texture_normal";
			break;
		}
	}

public:
	GLuint id = -1;
	std::string type;
	std::string path;
	int width = 0;
	int height = 0;
	GLenum format = 0;
	unsigned int mipmap = 0;
};