#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

using namespace std;

class Shader {
public:
	Shader(const char* vs, const char* fs, const char* gs = nullptr) {
		std::ifstream vs_file;
		std::ifstream fs_file;
		std::ifstream gs_file;

		vs_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		fs_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		gs_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		std::string vs_code;
		std::string fs_code;
		std::string gs_code;

		try
		{
			vs_file.open(vs);
			fs_file.open(fs);
			std::stringstream vs_stream;
			std::stringstream fs_stream;
			vs_stream << vs_file.rdbuf();
			fs_stream << fs_file.rdbuf();
			vs_code = vs_stream.str();
			fs_code = fs_stream.str();

			if (gs != nullptr) {
				gs_file.open(gs);
				std::stringstream gs_stream;
				gs_stream << gs_file.rdbuf();
				gs_code = gs_stream.str();
			}
		}
		catch (const std::exception&)
		{
			std::cout << "shader stream error" << std::endl;
		}

		const char* vs_code_str = vs_code.c_str();
		const char* fs_code_str = fs_code.c_str();
		GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs_id, 1, &vs_code_str, NULL);
		glCompileShader(vs_id);
		checkCompileErrors(vs_id, "vertex");

		GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs_id, 1, &fs_code_str, NULL);
		glCompileShader(fs_id);
		checkCompileErrors(fs_id, "fragment");

		GLuint gs_id;
		if (!gs_code.empty()) {
			const char* gs_code_str = gs_code.c_str();
			gs_id = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(gs_id, 1, &gs_code_str, NULL);
			glCompileShader(gs_id);
			checkCompileErrors(gs_id, "geometry");
		}

		GLuint prog = glCreateProgram();
		ID = prog;
		glAttachShader(prog, vs_id);
		glAttachShader(prog, fs_id);
		if (gs != nullptr) {
			glAttachShader(prog, gs_id);
		}
		glLinkProgram(prog);
		checkCompileErrors(prog, "program");

		glDeleteShader(vs_id);
		glDeleteShader(fs_id);
		if (gs != nullptr) {
			glDeleteShader(gs_id);
		}
	}

	void Use() {
		glUseProgram(ID);
	}

public:
	GLuint ID;

private:
	// utility function for checking shader compilation/linking errors.
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "program")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
