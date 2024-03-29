

#include "Application.h"
#include "../manager/TimeManager.h"
#include "../manager/InputManager.h"
#include "../graphics/Camera.h"
#include "../graphics/Model.h"
#include "Tree.h"
#include "GameObject.h"
#include "Component.h"
#include "../graphics/MeshRenderer.h"
#include "../graphics/Scene.h"
#include "../manager/SceneManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../script/Lua.h"
#include "../util/MFile.h"

GLFWwindow* Application::window;

Model* model0;

void Application::Init() {
    
    Application::InitWindow();

    InputManager::Init(window);

    //model0 = new Model(MFile::GetRes("objects/backpack/backpack.obj";
    new Camera(glm::vec3(0, 0, 8));
    //auto cam2 = new Camera(glm::vec3(8, 0, 3), -180);
    //cam2->ClearFlags = 0;
    //cam2->CullingMask = 0;
    auto s1 = SceneManager::LoadScene("s1");
    //s1->root->AddChild(model0->root);

    //Node* node = Lua::Test();
    //s1->root->AddChild(node);
    Lua::Test2();
    glDisable(GL_DEPTH_TEST);
}

void Application::Run()
{
    Update();
    EndFrame();
}

void Application::Update() {
    TimeManager::Update();
    InputManager::Update();

    Camera::Sort();
    Camera::Foreach([](Camera* cam) {
        cam->Update();

        auto root = Scene::gScene->root;
        Tree::Post(root, [](Node* node) {
            GameObject* game_object = dynamic_cast<GameObject*>(node);
            auto renderer = game_object->GetComponent<MeshRenderer>();
            if (renderer != nullptr) {
                renderer->Render();
            }
        });
    });

    InputManager::EndFrame();
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::EndFrame()
{
    
}

void Application::Exit() {
    glfwTerminate();
}

bool Application::ShouldClose() {
    return glfwWindowShouldClose(window);
}
