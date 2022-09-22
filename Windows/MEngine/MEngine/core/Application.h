#pragma once
#include <string>
using namespace std;

typedef std::string FilePath;

class Application {
public:
	static void Init(const FilePath& path);
	static FilePath GetPath(const FilePath& path);
	static FilePath GetRes(const FilePath& path);
public:
	static FilePath dataPath;

private:

};

void Application::Init(const FilePath& path) {
	dataPath = path;
}

FilePath Application::GetPath(const FilePath& path) {
	return dataPath + path;
}

FilePath Application::GetRes(const FilePath& path) {
	static const FilePath resPath = std::string("resources/");
	return dataPath + resPath + path;
}