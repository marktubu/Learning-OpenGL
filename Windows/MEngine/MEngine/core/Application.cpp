

#include "Application.h"
#include "../manager/TimeManager.h"
#include "../graphics/Camera.h"
#include "../graphics/Model.h"
#include "Tree.h"
#include "GameObject.h"
#include "../graphics/MeshRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLFWwindow* Application::window;

Camera* camera;
Model* model0;

void Application::Init() {
    Application::InitWindow();

    InputManager::Init(window);

    model0 = new Model("../resources/objects/backpack/backpack.obj");
    camera = new Camera(glm::vec3(0, 0, 5), glm::vec3(0, 0, -1));

    glEnable(GL_DEPTH_TEST);
}
bool init = false;

unsigned int cubeVAO, cubeVBO = 0;

void Application::Update() {
    TimeManager::Update();
    InputManager::Update();

    camera->Update();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // float time = glfwGetTime();
    // shader.setFloat("time", time);

    auto root = model0->root;
    Tree::Post(root, [](Node* node) {
        GameObject* game_object = dynamic_cast<GameObject *>(node);
        auto renderer = game_object->GetComponent<MeshRenderer>();
        if (renderer != nullptr) {
            renderer->Render();
        }
        });

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::Exit() {
    glfwTerminate();
}

bool Application::ShouldClose() {
    return glfwWindowShouldClose(window);
}
