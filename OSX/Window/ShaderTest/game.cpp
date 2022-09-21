/*******************************************************************
 ** This code is part of Breakout.
 **
 ** Breakout is free software: you can redistribute it and/or modify
 ** it under the terms of the CC BY 4.0 license as published by
 ** Creative Commons, either version 4 of the License, or (at your
 ** option) any later version.
 ******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "glm.hpp"

Game::Game(GLuint width, GLuint height)
: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    
}

Game::~Game()
{
    
}
SpriteRenderer  *Renderer;
void Game::Init()
{
    // 加载着色器
    ResourceManager::LoadShader("/Users/admin/OpenGL/LearningOpenGL/OSX/Window/ShaderTest/shaders/sprite.vs", "/Users/admin/OpenGL/LearningOpenGL/OSX/Window/ShaderTest/shaders/sprite.fs", nullptr, "sprite");
    // 配置着色器
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
                                      static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // 设置专用于渲染的控制
    Shader s = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(s);
    // 加载纹理
    ResourceManager::LoadTexture("/Users/admin/OpenGL/LearningOpenGL/OSX/Window/ShaderTest/awesomeface.png", GL_TRUE, "face");
}

void Game::Update(GLfloat dt)
{
    
}


void Game::ProcessInput(GLfloat dt)
{
    
}

void Game::Render()
{
    Texture2D t = ResourceManager::GetTexture("face");
    
    Renderer->DrawSprite(t,
                         glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}
