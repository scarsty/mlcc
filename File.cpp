#include "File.h"
#include <cstdio>
#include <ctime>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <strsafe.h>
#else
#include "dirent.h"
#include <sys/uio.h>
#include <unistd.h>
#endif

#ifdef __GNUC__
#include <sys/stat.h>
#include <sys/types.h>
#endif

File::File()
{
}

File::~File()
{
}

bool File::fileExist(const std::string& filename)
{
    if (filename.empty())
    {
        return false;
    }
    std::fstream file;
    bool ret = false;
    file.open(filename.c_str(), std::ios::in);
    if (file)
    {
        ret = true;
        file.close();
    }
    return ret;
}

void File::reverse(char* c, int n)
{
    for (int i = 0; i < n / 2; i++)
    {
        auto& a = *(c + i);
        auto& b = *(c + n - 1 - i);
        auto t = b;
        b = a;
        a = t;
    }
}

std::vector<char> File::readFile(const std::string& filename, int length)
{
    std::vector<char> s;
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        fprintf(stderr, "Cannot open file %s\n", filename.c_str());
        return s;
    }
    if (length <= 0)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, 0);
    }
    s.resize(length);
    fread(s.data(), 1, length, fp);
    fclose(fp);
    return s;
}

int File::readFile(const std::string& filename, void* s, int length)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        fprintf(stderr, "Cannot open file %s\n", filename.c_str());
        return 0;
    }
    int r = fread(s, 1, length, fp);
    fclose(fp);
    return r;
}

int File::writeFile(const std::string& filename, void* s, int length)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        fprintf(stderr, "Cannot write file %s\n", filename.c_str());
        return 0;
    }
    fseek(fp, 0, 0);
    fwrite(s, 1, length, fp);
    fclose(fp);
    return length;
}

std::vector<std::string> File::getFilesInPath(const std::string& path_name)
{
#ifdef _WIN32
    WIN32_FIND_DATAA ffd;
    //LARGE_INTEGER filesize;
    std::string szDir;
    //size_t length_of_arg;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    std::vector<std::string> ret;

    szDir = path_name + "\\*";
    hFind = FindFirstFileA(szDir.c_str(), &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        fprintf(stderr, "get file name error\n");
        return ret;
    }
    do
    {
        //f (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        std::string filename = ffd.cFileName;    //(const char*)
        if (filename != "." && filename != "..")
        {
            ret.push_back(filename);
        }
    } while (FindNextFileA(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        fprintf(stderr, "FindFirstFile error\n");
        return ret;
    }
    FindClose(hFind);
    return ret;
#else
    DIR* dir;
    struct dirent* ptr;
    dir = opendir(path_name.c_str());
    std::vector<std::string> ret;
    while ((ptr = readdir(dir)) != NULL)
    {
        std::string filename = std::string(ptr->d_name);
        if (filename != "." && filename != "..")
        {
            ret.push_back(filename);
        }
    }
    closedir(dir);
    //std::sort(ret.begin(), ret.end());
    return ret;
#endif
}

std::vector<std::string> File::getFilesRecursiveInPath(const std::string& path_name)
{
    std::vector<std::string> files, paths;
    paths.push_back(path_name);
    while (!paths.empty())
    {
        auto p = paths.back();
        paths.pop_back();
        if (isPath(p))
        {
            auto new_paths = getFilesInPath(p);
            for (auto& np : new_paths)
            {
                paths.push_back(p + "/" + np);
            }
        }
        else
        {
            files.push_back(p);
        }
    }
    return files;
}

bool File::isPath(const std::string& name)
{
    if (!fileExist(name))
    {
        return false;
    }
#ifdef _WIN32
    return GetFileAttributesA(name.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
#else
    struct stat sb;
    stat(name.c_str(), &sb);
    return sb.st_mode & S_IFDIR;
#endif
}

std::string File::getFileTime(std::string filename)
{
#if defined(__clang__) && defined(_WIN32)
    struct __stat64 s;
    int sss = __stat64(filename.c_str(), &s);
#else
    struct stat s;
    int sss = stat(filename.c_str(), &s);
#endif
    struct tm* filedate = NULL;
    time_t tm_t = 0;
    uint32_t dos_date;
    if (sss == 0)
    {
        tm_t = s.st_mtime;
        filedate = localtime(&tm_t);
        char buf[128] = { 0 };
        strftime(buf, 64, "%Y-%m-%d  %H:%M:%S", filedate);
        fprintf(stdout, "%s:%s\n", filename.c_str(), buf);
        return buf;
    }
    return "";
}

void File::changePath(const std::string& path)
{
    chdir(path.c_str());
}

size_t File::getLastPathCharPos(const std::string& filename)
{
    //here use std::string::npos == (decltype(std::string::npos))(-1)
    //it seems right
    size_t pos = std::string::npos;
#ifdef _WIN32
    //ansi
    for (int i = 0; i < filename.size(); i++)
    {
        if (uint8_t(filename[i]) >= 128)
        {
            i++;
        }
        else if (filename[i] == '\\' || filename[i] == '/')
        {
            pos = i;
        }
    }
#else
    pos = filename.find_last_of('/');
#endif    // _WIN32
    return pos;
}

std::string File::getFileExt(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    auto pos_d = filename.find_last_of('.');
    if (pos_d != std::string::npos && (pos_p < pos_d || pos_p == std::string::npos))
    {
        return filename.substr(pos_d + 1);
    }
    return "";
}

//find the last point as default, and find the first when mode is 1
std::string File::getFileMainname(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    auto pos_d = filename.find_last_of('.');
    if (pos_d != std::string::npos && (pos_p < pos_d || pos_p == std::string::npos))
    {
        return filename.substr(0, pos_d);
    }
    return filename;
}

std::string File::getFilenameWithoutPath(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    if (pos_p != std::string::npos)
    {
        return filename.substr(pos_p + 1);
    }
    return filename;
}

std::string File::changeFileExt(const std::string& filename, const std::string& ext)
{
    auto e = ext;
    if (!e.empty() && e[0] != '.')
    {
        e = "." + e;
    }
    return getFileMainname(filename) + e;
}

std::string File::getFilePath(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    if (pos_p != std::string::npos)
    {
        return filename.substr(0, pos_p);
    }
    return "";
}
