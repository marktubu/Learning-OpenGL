#pragma once

#include "app.h"

class Blending : public App {
public:
	void Run() override;
};

void Blending::Run() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("../shader/coord.vs", "../shader/coord.fs");

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    };

    unsigned int indices[] = {
        0, 2, 1,
        2, 1, 3,
    };

    // positions all containers
    glm::vec3 cubePositions[] = {
       glm::vec3(0.0f,  0.0f,  0.0f),
       glm::vec3(2.0f,  5.0f, -15.0f),
       glm::vec3(-1.5f, -2.2f, -2.5f),
       glm::vec3(-3.8f, -2.0f, -12.3f)
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3*sizeof(float)));

    stbi_set_flip_vertically_on_load(true);
    unsigned int texture = loadTexture("../texture/grass.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    unsigned int texture2 = loadTexture("../texture/blending_transparent_window.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render loop
    // -----------
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

        glBindVertexArray(VAO);

        lightingShader.use();
        lightingShader.setInt("texture1", 0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE0);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

        glBindTexture(GL_TEXTURE_2D, texture2);
        lightingShader.use();
        lightingShader.setInt("texture1", 0);

        glActiveTexture(GL_TEXTURE0);

        // view/projection transformations
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, 0.3f, 1.f));
        lightingShader.setMat4("model", model);

        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
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