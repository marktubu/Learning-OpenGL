#pragma once

#include <string>

#include <assimp/types.h>

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../util/stb_image.h"

using namespace std;

class Texture {
public:
	static Texture* Create(std::string path) {
		Texture* tex = new Texture();
		Texture::Fill(tex, path);
		return tex;
	}

	static void Fill(Texture* tex, std::string path) {
		if (tex->id > 0) {
			glDeleteTextures(1, &tex->id);
		}
		int width, height, channel;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channel, 0);

		glGenTextures(1, &tex->id);
		glBindTexture(GL_TEXTURE_2D, tex->id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum format = 0;
		switch (channel) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		}

		tex->width = width;
		tex->height = height;
		tex->format = format;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	void Load(std::string path) {
		Texture::Fill(this, path);
	}

public:
	GLuint id;
	string type;
	aiString path;
	int width;
	int height;
	GLenum format;
	unsigned int mipmap;
};