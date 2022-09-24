

#include "Application.h"
#include "../manager/TimeManager.h"
#include "../graphics/Camera.h"
#include "../graphics/Model.h"

GLFWwindow* Application::window;
FilePath Application::dataPath;

Shader* shader;
Camera* camera;
Model* model0;

void Application::Init(const FilePath& path) {
    dataPath = path;
    Application::InitWindow();

    InputManager::Init(window);

    shader = new Shader("../resources/shader/geo.vs"
        , "../resources/shader/geo.fs"
        , "../resources/shader/geo.gs");

    model0 = new Model("../resources/objects/backpack/backpack.obj");
    camera = new Camera(glm::vec3(0, 0, 5), glm::vec3(0, 0, -1));
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

    glm::mat4 model = glm::mat4(1.0);
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->Use();
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    model0->Draw(*shader);

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

FilePath Application::GetPath(const FilePath& path) {
    return dataPath + path;
}

FilePath Application::GetRes(const FilePath& path) {
    static const FilePath resPath = std::string("resources/");
    return dataPath + resPath + path;
}