#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum MoveDir {
	Front,
	Back,
	Left,
	Right,
};

const float MoveSpeed = 1.f;
const float Sensitivity = 0.1f;

class Camera {
public:
	Camera();

	glm::mat4 GetViewMatrix();

	void ProcessMove(MoveDir dir) {

	}

	void ProcessScroll() {

	}

public:
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

private:
	float Yaw;
	float Roll;
	float Pitch;
};