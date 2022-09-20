#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.f;
const float PITCH = 0.f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.f;

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f),
		float yaw = YAW, float pitch = PITCH) :
		Front(glm::vec3(0.f, 0.f, -1.f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
		Front(glm::vec3(0.f, 0.f, -1.f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		//return glm::lookAt(Position, Position + Front, Up);
		return lookAt();
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= RIGHT * velocity;
		if (direction == RIGHT)
			Position += RIGHT * velocity;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch) {
			if (Pitch > 89.f)
				Pitch = 89.f;
			if (Pitch < -89.f)
				Pitch = -89.f;
		}

		updateCameraVectors();
	}

	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.f)
			Zoom = 1.f;
		if (Zoom > 45.f)
			Zoom = 45.f;
	}

private:
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

	// OpenGL is Right-Handed-Coordinate-System
	// glm::mat4 ÊÇ column based matrix
	glm::mat4 lookAt() {
		glm::vec3 forward = -Front;
		glm::vec3 right = glm::normalize(glm::cross(WorldUp, forward));
		glm::vec3 up = glm::normalize(glm::cross(forward, right));

		glm::mat4 m(0.0);
		glm::mat3 rot_m;
		m[0][0] = rot_m[0][0] = right.x;
		m[1][0] = rot_m[1][0] = right.y;
		m[2][0] = rot_m[2][0] = right.z;

		m[0][1] = rot_m[0][1] = up.x;
		m[1][1] = rot_m[1][1] = up.y;
		m[2][1] = rot_m[2][1] = up.z;

		m[0][2] = rot_m[0][2] = forward.x;
		m[1][2] = rot_m[1][2] = forward.y;
		m[2][2] = rot_m[2][2] = forward.z;

		glm::vec3 mv = -rot_m * Position;

		m[3][0] = mv.x;
		m[3][1] = mv.y;
		m[3][2] = mv.z;
		m[3][3] = 1.0;

		return m;
	}

};