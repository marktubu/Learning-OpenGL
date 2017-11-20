#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <istream>

#include <SOIL.h>

std::map<std::string, Texture2D>  ResourceManager::Textures;
std::map<std::string, Shader>	  ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const GLchar* vShaderFile, const GLchar *fShaderFile, const GLchar* gShaderFile, const std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const GLchar* file, GLboolean alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear()
{
	for (auto iter : Shaders)
	{
		glDeleteProgram(iter.second.ID);
	}

	for (auto iter : Textures)
	{
		glDeleteTextures(1, &iter.second.ID);
	}
}

Shader ResourceManager::loadShaderFromFile(const GLchar* vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;

	try 
	{
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << 
	}
}