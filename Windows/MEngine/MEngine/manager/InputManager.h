#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class InputManager {
public:
	static void Init(GLFWwindow* _window);
	static void Update();

private:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
	static GLFWwindow* window;
};
