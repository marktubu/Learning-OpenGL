#include "TimeManager.h"

float TimeManager::Delta = 0.f;
float TimeManager::lastFrameTime = -1.f;

void TimeManager::Update()
{
	float time = glfwGetTime();
	if (lastFrameTime < 0.f) {
		lastFrameTime = time;
	}

	Delta = time - lastFrameTime;
	lastFrameTime = time;
}
