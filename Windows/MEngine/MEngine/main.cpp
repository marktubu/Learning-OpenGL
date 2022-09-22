
#include "core/Application.h"

int main() {
	std::string path = "C:\\File\\Project\\Unity\\Learning-OpenGL\\Windows\\MEngine\\";
	Application::Init(path);

	while (!Application::ShouldClose()) {
		Application::Update();
	}

	Application::Exit();
	return 0;
}