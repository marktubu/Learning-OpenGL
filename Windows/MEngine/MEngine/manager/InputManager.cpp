
#include <glad/glad.h>

#include "InputManager.h"
#include "../util/Screen.h"

float lastX = Screen::Width / 2.0f;
float lastY = Screen::Height / 2.0f;
bool firstMouse = true;

GLFWwindow* InputManager::window;
float InputManager::MouseMoveXOffset;
float InputManager::MouseMoveYOffset;
float InputManager::MouseScrollXOffset;
float InputManager::MouseScrollYOffset;

void InputManager::Init(GLFWwindow* _window) {
    window = _window;
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void InputManager::Update() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void InputManager::EndFrame()
{
    MouseMoveXOffset = 0;
    MouseMoveYOffset = 0;
    MouseScrollXOffset = 0;
    MouseScrollYOffset = 0;
}

bool InputManager::GetKey(int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void InputManager::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void InputManager::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    MouseMoveXOffset = xoffset;
    MouseMoveYOffset = yoffset;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    MouseScrollXOffset = xoffset;
    MouseScrollYOffset = yoffset;
}