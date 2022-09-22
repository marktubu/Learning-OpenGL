

#include "Application.h"

GLFWwindow* Application::window;
FilePath Application::dataPath;

Shader* shader;

void Application::Init(const FilePath& path) {
    dataPath = path;
    Application::InitWindow();

    InputManager::Init(window);

    shader = new Shader("../resources/shader/antialiasing.vert"
        , "../resources/shader/antialiasing.frag");

}
bool init = false;

unsigned int cubeVAO, cubeVBO = 0;

void Application::Update() {
    InputManager::Update();
    
    if (!init) {
        init = true;
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_MULTISAMPLE); // enabled by default on some drivers, but not all so always enable to make sure

        // build and compile shaders
        // -------------------------
        /*Shader s = Shader::Create("../resources/shader/antialiasing.vert"
            , "../resources/shader/antialiasing.frag");
        shader = &s;*/

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float cubeVertices[] = {
            // positions       
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

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
    /*glm::mat4 projection(1.0);
    shader.setMat4("projection", projection);
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("model", glm::mat4(1.0f));*/

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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