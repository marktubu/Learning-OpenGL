
#include <algorithm>

#include "Camera.h"

#include "../manager/InputManager.h"
#include "../manager/TimeManager.h"
#include "../util/Screen.h"

Camera* Camera::Current = nullptr;
std::vector<Camera*> Camera::Cameras;


void Camera::SetCurrent(Camera* camera) {
	Camera::Current = camera;
}

void Camera::Sort() {
	std::sort(Cameras.begin(), Cameras.end(), [](Camera* a, Camera* b) {
		return a->Depth < b->Depth;
	});
}

void Camera::Foreach(std::function<void(Camera*)> func)
{
	for (auto i = Cameras.begin();i != Cameras.end();i++) {
		auto cam = *i;
		SetCurrent(cam);
		cam->Clear();
		func(cam);
	}
}

void Camera::Clear()
{
	glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a);
	glClear(ClearFlags);
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix() {
	return glm::perspective(glm::radians(FOV), (float)Screen::Width / Screen::Height, Near, Far);
}

void Camera::ProcessMove() {
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

void Camera::ProcessMouse() {
	float lr = InputManager::MouseMoveXOffset * Sensitivity;
	float fb = InputManager::MouseMoveYOffset * Sensitivity;

	Pitch += fb;
	Yaw += lr;
	if (Pitch < -89) Pitch = -89;
	if (Pitch > 89) Pitch = 89;

	updateCamera();
}

void Camera::updateCamera() {
	glm::vec3 dir;
	dir.x = glm::cos(glm::radians(Pitch)) * glm::cos(glm::radians(Yaw));
	dir.y = glm::sin(glm::radians(Pitch));
	dir.z = glm::cos(glm::radians(Pitch)) * glm::sin(glm::radians(Yaw));
	Front = glm::normalize(dir);

	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}
