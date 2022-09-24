

#include "Application.h"
#include "../manager/TimeManager.h"
#include "../graphics/Camera.h"

GLFWwindow* Application::window;
FilePath Application::dataPath;

Shader* shader;
Camera* camera;

void Application::Init(const FilePath& path) {
    dataPath = path;
    Application::InitWindow();

    InputManager::Init(window);

    shader = new Shader("../resources/shader/geo.vs"
        , "../resources/shader/geo.fs"
        , "../resources/shader/geo.gs");

    camera = new Camera(glm::vec3(0, 0, 5), glm::vec3(0, 0, -1));
}
bool init = false;

unsigned int cubeVAO, cubeVBO = 0;

void Application::Update() {
    TimeManager::Update();
    InputManager::Update();

    camera->Update();
    
    if (!init) {
        init = true;
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_MULTISAMPLE); // enabled by default on some drivers, but not all so always enable to make sure

        // build and compile shaders
        // -------------------------

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float cubeVertices[] = {
            // positions       
            -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f

        };
        // setup cube VAO

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set transformation matrices		
    shader->Use();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    shader->setMat4("projection", projection);
    shader->setMat4("view", camera->GetViewMatrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, 30.f, glm::vec3(glm::radians(30.f), glm::radians(30.f), 0));
    shader->setMat4("model", model);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

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