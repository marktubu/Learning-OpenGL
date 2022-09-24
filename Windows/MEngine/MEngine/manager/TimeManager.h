#pragma once

#include <GLFW/glfw3.h>

class TimeManager {
public:
	static void Update();

public:
	static float Delta;

private:
	static float lastFrameTime;
};