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

	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void bindUniformBlock(const GLchar* name, GLuint bind_point) {
		GLuint indices = glGetUniformBlockIndex(ID, name);
		glUniformBlockBinding(ID, indices, bind_point);
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
