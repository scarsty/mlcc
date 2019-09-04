#pragma once
#include <cstring>
#include <string>
#include <vector>

class File
{
private:
    File();
    virtual ~File();

public:
    //read and write file
    static bool fileExist(const std::string& filename);
    static void reverse(char* c, int n);

    static std::vector<char> readFile(const std::string& filename, int length = -1);
    static int readFile(const std::string& filename, void* s, int length);
    static int writeFile(const std::string& filename, void* s, int length);

    template <class T>
    static void readDataToVector(const char* data, int length, std::vector<T>& v)
    {
        readDataToVector(data, length, v, sizeof(T));
    }

    template <class T>
    static void readDataToVector(const char* data, int length, std::vector<T>& v, int length_one)
    {
        int count = length / length_one;
        v.resize(count);
        for (int i = 0; i < count; i++)
        {
            memcpy(&v[i], data + length_one * i, length_one);
        }
    }

    template <class T>
    static void readFileToVector(const std::string& filename, std::vector<T>& v)
    {
        auto buffer = readFile(filename);
        readDataToVector(buffer.data(), buffer.size(), v);
    }

    template <class T>
    static void writeVectorToData(char* data, int length, std::vector<T>& v, int length_one)
    {
        int count = length / length_one;
        v.resize(count);
        for (int i = 0; i < count; i++)
        {
            memcpy(data + length_one * i, &v[i], length_one);
        }
    }

    //other file operations
    static std::vector<std::string> getFilesInPath(const std::string& path_name, int recursive = 0, int include_path = 0);
    static bool isPath(const std::string& name);
    static std::string getFileTime(std::string filename);
    static void changePath(const std::string& path);

    //functions about filename

private:
    static size_t getLastPathCharPos(const std::string& filename);

public:
    //functions about file name
    static std::string getFileExt(const std::string& filename);
    static std::string getFileMainname(const std::string& fileName);
    static std::string getFilenameWithoutPath(const std::string& fileName);
    static std::string changeFileExt(const std::string& filename, const std::string& ext);
    static std::string getFilePath(const std::string& filename);
};
