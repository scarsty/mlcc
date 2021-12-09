#pragma once
#include <cstring>
#include <string>
#include <vector>

namespace File
{

//read and write file
bool fileExist(const std::string& name);
bool pathExist(const std::string& name);

void reverse(char* c, int n);

std::vector<char> readFile(const std::string& filename, int length = -1);
int readFile(const std::string& filename, void* s, int length);
int writeFile(const std::string& filename, void* s, int length);

template <class T>
void readDataToVector(const char* data, int length, std::vector<T>& v, int length_one)
{
    int count = length / length_one;
    v.resize(count);
    for (int i = 0; i < count; i++)
    {
        memcpy(&v[i], data + length_one * i, length_one);
    }
}

template <class T>
void readDataToVector(const char* data, int length, std::vector<T>& v)
{
    readDataToVector(data, length, v, sizeof(T));
}

template <class T>
void readFileToVector(const std::string& filename, std::vector<T>& v)
{
    auto buffer = readFile(filename);
    readDataToVector(buffer.data(), buffer.size(), v);
}

template <class T>
void writeVectorToData(char* data, int length, std::vector<T>& v, int length_one)
{
    int count = length / length_one;
    v.resize(count);
    for (int i = 0; i < count; i++)
    {
        memcpy(data + length_one * i, &v[i], length_one);
    }
}

//other file operations
std::vector<std::string> getFilesInPath(const std::string& pathname, int recursive = 0, int include_path = 0);
std::string getFileTime(std::string filename);
void changePath(const std::string& path);
void makePath(const std::string& path);

//functions about file name
std::string getFileExt(const std::string& filename);
std::string getFileMainname(const std::string& filename);
std::string getFilenameWithoutPath(const std::string& filename);
std::string changeFileExt(const std::string& filename, const std::string& ext);
std::string getFilePath(const std::string& filename);
std::string toLegalFileanme(const std::string& filename, int allow_path = 1);

}    // namespace File
