#pragma once

#include "app.h"

class Geometry : public App {
public:
    void Run() override;
};

void Geometry::Run() {
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader shader(FileSystem::getPath("resources/shader/geo.vs").c_str()
        , FileSystem::getPath("resources/shader/geo.fs").c_str()
        , FileSystem::getPath("resources/shader/geo.gs").c_str());

    Shader normal_shader(FileSystem::getPath("resources/shader/geo_normal.vs").c_str()
        , FileSystem::getPath("resources/shader/geo_normal.fs").c_str()
        , FileSystem::getPath("resources/shader/geo_normal.gs").c_str());

    Model* model0 = new Model(FileSystem::getPath("resources/objects/backpack/backpack.obj").c_str());

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // float time = glfwGetTime();
        // shader.setFloat("time", time);

        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        model0->Draw(shader);

        normal_shader.use();
        normal_shader.setMat4("model", model);
        normal_shader.setMat4("view", view);
        normal_shader.setMat4("projection", projection);

        model0->Draw(normal_shader);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}