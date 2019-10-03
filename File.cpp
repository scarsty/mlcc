#include "File.h"
#include <algorithm>
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

std::vector<std::string> File::getFilesInPath(const std::string& path, int recursive, int include_path)
{
    if (recursive == 0)
    {
#ifdef _WIN32
        WIN32_FIND_DATAA ffd;
        //LARGE_INTEGER filesize;
        std::string szDir;
        //size_t length_of_arg;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        DWORD dwError = 0;
        std::vector<std::string> ret;

        szDir = path + "\\*";
        hFind = FindFirstFileA(szDir.c_str(), &ffd);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            fprintf(stderr, "Get files error in %s\n", path.c_str());
            return ret;
        }
        do
        {
            //f (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            std::string filename = ffd.cFileName;    //(const char*)
            if (filename != "." && filename != ".." && (include_path != 0 || (include_path == 0 && !isPath(path + "/" + filename))))
            {
                ret.push_back(filename);
            }
        } while (FindNextFileA(hFind, &ffd) != 0);

        dwError = GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
        {
            fprintf(stderr, "Find first file error\n");
            return ret;
        }
        FindClose(hFind);
#else
        DIR* dir;
        struct dirent* ptr;
        dir = opendir(path.c_str());
        std::vector<std::string> ret;
        if (dir == nullptr)
        {
            fprintf(stderr, "Get files error in %s\n", path.c_str());
            return ret;
        }
        while ((ptr = readdir(dir)) != nullptr)
        {
            std::string filename = std::string(ptr->d_name);
            if (filename != "." && filename != ".." && (include_path != 0 || (include_path == 0 && !isPath(path + "/" + filename))))
            {
                ret.push_back(filename);
            }
        }
        closedir(dir);
        //std::sort(ret.begin(), ret.end());
#endif
        return ret;
    }
    else
    {
        std::vector<std::string> files, paths;
        paths = getFilesInPath(path, 0, 1);
        while (!paths.empty())
        {
            auto p = paths.back();
            paths.pop_back();
            if (isPath(path + "/" + p))
            {
                auto new_paths = getFilesInPath(path + "/" + p, 0, 1);
                for (auto& np : new_paths)
                {
                    paths.push_back(p + "/" + np);
                }
                if (include_path)
                {
                    files.push_back(p);
                }
            }
            else
            {
                files.push_back(p);
            }
        }
        std::reverse(files.begin(), files.end());
        return files;
    }
}

bool File::isPath(const std::string& name)
{
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
