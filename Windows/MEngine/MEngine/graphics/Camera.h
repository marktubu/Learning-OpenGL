#pragma once

#include <vector>
#include <functional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum MoveDir {
	Front,
	Back,
	Left,
	Right,
};

const float MoveSpeed = 1.f;
const float Sensitivity = 0.01f;

class Camera {
public:
	Camera(glm::vec3 pos, float yaw = -90.f) : Position(pos), 
		Yaw(yaw), Pitch(0.0), WorldUp(glm::vec3(0, 1, 0)),
		FOV(45), Near(0.1f), Far(100.f), 
		ClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT),
		ClearColor(glm::u8vec4(0))
	{
		updateCamera();
		Cameras.push_back(this);
	}

	static void SetCurrent(Camera* camera);

	static void Foreach(std::function<void(Camera*)> func);

	void Clear();

	glm::mat4 GetViewMatrix();

	glm::mat4 GetProjectionMatrix();

	void Update() {
		ProcessMove();
		ProcessMouse();
	}

	void ProcessMove();

	void ProcessMouse();

public:
	static Camera* Current;
	static std::vector<Camera*> Cameras;

	glm::vec3 Position;

	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float FOV;
	float Near;
	float Far;

	int ClearFlags;
	glm::u8vec4 ClearColor;

private:
	void updateCamera();

private:
	float Yaw;
	float Pitch;
};
