
#include "core/Application.h"
#include "util/MFile.h"

int main() {
	std::string path = "C:\\File\\Project\\Unity\\Learning-OpenGL\\Windows\\MEngine\\";
	MFile::Init(path);

	Application::Init();

	while (!Application::ShouldClose()) {
		Application::Run();
	}

	Application::Exit();
	return 0;
}