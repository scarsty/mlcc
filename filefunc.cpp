#include "filefunc.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <strsafe.h>
#else
#include "dirent.h"
#ifndef __APPLE__
#include <sys/io.h>
#endif
#include <sys/uio.h>
#include <unistd.h>
#define _mkdir(p) mkdir(p, S_IRWXU)
#endif

#ifdef __GNUC__
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef _MSC_VER
#define CPP_VERSION _MSVC_LANG
#else
#define CPP_VERSION __cplusplus
#endif

#if CPP_VERSION >= 201703L
#include <filesystem>
#endif

bool filefunc::fileExist(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
    auto exist = ::GetFileAttributesExA(name.c_str(), ::GetFileExInfoStandard, &attrs);
    if (exist)
    {
        return !(attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    }
    return false;
#else
    if (access(name.c_str(), 0) == -1)
    {
        return false;
    }
    struct stat sb;
    stat(name.c_str(), &sb);
    return !(sb.st_mode & S_IFDIR);
#endif
}

bool filefunc::pathExist(const std::string& name)
{
    if (name.empty())
    {
        return false;
    }
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
    ::GetFileAttributesExA(name.c_str(), ::GetFileExInfoStandard, &attrs);
    return attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    if (access(name.c_str(), 0) == -1)
    {
        return false;
    }
    struct stat sb;
    stat(name.c_str(), &sb);
    return sb.st_mode & S_IFDIR;
#endif
}

void filefunc::reverse(char* c, int n)
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

std::vector<char> filefunc::readFile(const std::string& filename, int length)
{
    std::vector<char> s;
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        //fprintf(stderr, "Cannot open file %s\n", filename.c_str());
        return s;
    }
    if (length <= 0)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, 0);
    }
    s.resize(length);
    if (fread(s.data(), 1, length, fp) < length)
    {
        //fprintf(stderr, "Read file %s unfinished!\n", filename.c_str());
    }
    fclose(fp);
    return s;
}

int filefunc::readFile(const std::string& filename, void* s, int length)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        //fprintf(stderr, "Cannot open file %s\n", filename.c_str());
        return 0;
    }
    int r = fread(s, 1, length, fp);
    fclose(fp);
    return r;
}

int filefunc::writeFile(const std::string& filename, void* s, int length)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp)
    {
        //fprintf(stderr, "Cannot write file %s\n", filename.c_str());
        return 0;
    }
    fseek(fp, 0, 0);
    fwrite(s, 1, length, fp);
    fclose(fp);
    return length;
}

std::vector<std::string> filefunc::getFilesInPath(const std::string& pathname, int recursive /*= 0*/, int include_path /*= 0*/)
{
    if (recursive == 0)
    {
#ifdef _WIN32
        WIN32_FIND_DATAA find_data;
        //LARGE_INTEGER filesize;
        std::string dir;
        HANDLE h_find = INVALID_HANDLE_VALUE;
        DWORD error = 0;
        std::vector<std::string> files;

        dir = pathname + "\\*";
        h_find = FindFirstFileA(dir.c_str(), &find_data);

        if (h_find == INVALID_HANDLE_VALUE)
        {
            //fprintf(stderr, "Get files error in %s\n", pathname.c_str());
            return files;
        }
        do
        {
            std::string filename = find_data.cFileName;    //(const char*)
            if (filename != "." && filename != ".." && (include_path != 0 || (include_path == 0 && !pathExist(pathname + "/" + filename))))
            {
                files.push_back(filename);
            }
        } while (FindNextFileA(h_find, &find_data) != 0);

        error = GetLastError();
        if (error != ERROR_NO_MORE_FILES)
        {
            //fprintf(stderr, "Find first file error\n");
            return files;
        }
        FindClose(h_find);
#else
        DIR* dir;
        struct dirent* ptr;
        dir = opendir(pathname.c_str());
        std::vector<std::string> files;
        if (dir == nullptr)
        {
            fprintf(stderr, "Get files error in %s\n", pathname.c_str());
            return files;
        }
        while ((ptr = readdir(dir)) != nullptr)
        {
            std::string filename = std::string(ptr->d_name);
            if (filename != "." && filename != ".." && (include_path != 0 || (include_path == 0 && !pathExist(pathname + "/" + filename))))
            {
                files.push_back(filename);
            }
        }
        closedir(dir);
        //std::sort(ret.begin(), ret.end());
#endif
        return files;
    }
    else
    {
        std::vector<std::string> files, paths;
        paths = getFilesInPath(pathname, 0, 1);
        while (!paths.empty())
        {
            auto p = paths.back();
            paths.pop_back();
            if (pathExist(pathname + "/" + p))
            {
                auto new_paths = getFilesInPath(pathname + "/" + p, 0, 1);
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

void filefunc::makePath(const std::string& path)
{
    std::vector<std::string> paths;
    auto p = path;
    while (true)
    {
        if (pathExist(p) || p == "")
        {
            break;
        }
        paths.push_back(p);
        p = getFilePath(p);
    }
    for (auto it = paths.rbegin(); it != paths.rend(); it++)
    {
        _mkdir(it->c_str());
    }
}

std::string filefunc::getFileTime(std::string filename)
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
        //fprintf(stdout, "%s: %s\n", filename.c_str(), buf);
        return buf;
    }
    return "";
}

void filefunc::changePath(const std::string& path)
{
    if (chdir(path.c_str()) != 0)
    {
        fprintf(stderr, "Failed to change work path %s\n", path.c_str());
    }
}

static size_t getLastPathCharPos(const std::string& filename, int utf8 = 0)
{
    //here use std::string::npos == (decltype(std::string::npos))(-1)
    //it seems right
    size_t pos = std::string::npos;
#ifdef _WIN32
    //ansi
    for (int i = 0; i < filename.size(); i++)
    {
        if (utf8 == 0 && uint8_t(filename[i]) >= 128)
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

std::string filefunc::getFileExt(const std::string& filename)
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
std::string filefunc::getFileMainname(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    auto pos_d = filename.find_last_of('.');
    if (pos_d != std::string::npos && (pos_p < pos_d || pos_p == std::string::npos))
    {
        return filename.substr(0, pos_d);
    }
    return filename;
}

std::string filefunc::getFilenameWithoutPath(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    if (pos_p != std::string::npos)
    {
        return filename.substr(pos_p + 1);
    }
    return filename;
}

std::string filefunc::changeFileExt(const std::string& filename, const std::string& ext)
{
    auto e = ext;
    if (!e.empty() && e[0] != '.')
    {
        e = "." + e;
    }
    return getFileMainname(filename) + e;
}

//when a path name ends with "/", the result may be not good
std::string filefunc::getFilePath(const std::string& filename)
{
    auto pos_p = getLastPathCharPos(filename);
    if (pos_p != std::string::npos)
    {
        return filename.substr(0, pos_p);
    }
    return "";
}

std::string filefunc::toLegalFileanme(const std::string& filename, int allow_path)
{
    std::string f = filename, chars = " *<>?|";
    if (!allow_path)
    {
        chars += ":/\\";
    }
    for (char i = 31; i >= 0; i--)
    {
        chars += i;
    }
    size_t pos = 0;
    while ((pos = f.find_first_of(chars, pos)) != std::string::npos)
    {
        f[pos] = '_';
    }
    return f;
}

bool filefunc::compareNature(const std::string& a, const std::string& b)
{
    if (a.empty() && b.empty()) { return false; }
    if (a.empty()) { return true; }
    if (b.empty()) { return false; }
    if (std::isdigit(a[0]) && !std::isdigit(b[0])) { return true; }
    if (!std::isdigit(a[0]) && std::isdigit(b[0])) { return false; }
    if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
    {
        if (std::toupper(a[0]) == std::toupper(b[0])) { return compareNature(a.substr(1), b.substr(1)); }
        {
            return (std::toupper(a[0]) < std::toupper(b[0]));
        }
    }

    // Both strings begin with digit --> parse both numbers
    std::istringstream issa(a);
    std::istringstream issb(b);
    int ia, ib;
    issa >> ia;
    issb >> ib;
    if (ia != ib) { return ia < ib; }

    // Numbers are the same --> remove numbers and recurse
    std::string anew, bnew;
    std::getline(issa, anew);
    std::getline(issb, bnew);
    return (compareNature(anew, bnew));
}
