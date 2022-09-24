#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../manager/InputManager.h"
#include "../manager/TimeManager.h"

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
	Camera(glm::vec3 pos, glm::vec3 front) : Position(pos), Front(front), WorldUp(glm::vec3(0,1,0)), Yaw(-90.0), Pitch(0.0) {
		updateCamera();
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(Position, Position + Front, Up);
	}

	void Update() {
		ProcessMove();
		ProcessMouse();
	}

	void ProcessMove() {
		float delta = TimeManager::Delta;
		float fb_move = 0.0f;
		float lr_move = 0.0f;
		float move = delta * MoveSpeed;
		if (InputManager::GetKey(GLFW_KEY_W))
			fb_move += move;
		if (InputManager::GetKey(GLFW_KEY_S))
			fb_move -= move;
		if (InputManager::GetKey(GLFW_KEY_D))
			lr_move += move;
		if (InputManager::GetKey(GLFW_KEY_A))
			lr_move -= move;
		
		Position += Front * fb_move + Right * lr_move;
	}

	void ProcessMouse() {
		glm::vec3 dir;
		float lr = InputManager::MouseMoveXOffset * Sensitivity;
		float fb = InputManager::MouseMoveYOffset * Sensitivity;
		InputManager::MouseMoveXOffset = 0;
		InputManager::MouseMoveYOffset = 0;

		Pitch += fb;
		Yaw += lr;
		if (Pitch < -89) Pitch = -89;
		if (Pitch > 89) Pitch = 89;

		dir.x = glm::cos(glm::radians(Pitch)) * glm::cos(glm::radians(Yaw));
		dir.y = glm::sin(glm::radians(Pitch));
		dir.z = glm::cos(glm::radians(Pitch)) * glm::sin(glm::radians(Yaw));

		Front = glm::normalize(dir);
		updateCamera();
	}

public:
	glm::vec3 Position;

	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

private:
	void updateCamera() {
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

private:
	float Yaw;
	float Pitch;
};