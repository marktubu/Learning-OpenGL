#pragma once

#include <string>

typedef std::string FilePath;

class MFile {
public:
	static void Init(const FilePath& path);

	static FilePath GetPath(const FilePath& path);
	static FilePath GetRes(const FilePath& path);

public:
	static FilePath dataPath;
};