#include "MFile.h"

FilePath MFile::dataPath;

void MFile::Init(const FilePath& path) {
    dataPath = path;
}

FilePath MFile::GetPath(const FilePath& path) {
    return dataPath + path;
}

FilePath MFile::GetRes(const FilePath& path) {
    static const FilePath resPath = std::string("resources/");
    return dataPath + resPath + path;
}