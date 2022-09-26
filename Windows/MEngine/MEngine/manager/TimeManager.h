#pragma once

class TimeManager {
public:
	static void Update();

public:
	static float Delta;

private:
	static float lastFrameTime;
};