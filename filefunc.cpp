#include "filefunc.h"

#include <cctype>
#include <cstdio>
#include <ctime>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <strsafe.h>
#define localtime _localtime64
#define stat _stat64
#else
#include "dirent.h"
#if !defined(__APPLE__) && !defined(__ANDROID__)
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

namespace
{
std::string remove_trailing_path_chars(const std::string& input)
{
    if (input.empty())
    {
        return input;
    }
    std::string path = input;
    while (path.size() > 1 && filefunc::is_path_char(path.back()))
    {
#ifdef _WIN32
        if (path.size() == 3 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':')
        {
            break;
        }
#endif
        path.pop_back();
    }
    return path;
}

std::string normalize_path(std::string path)
{
#ifdef _WIN32
    for (auto& ch : path)
    {
        if (ch == '/')
        {
            ch = '\\';
        }
    }
#endif
    return path;
}

bool is_absolute_path(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }
#ifdef _WIN32
    if (path.size() >= 3 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
    {
        return true;
    }
    return path.size() >= 2 && (path[0] == '\\' || path[0] == '/') && (path[1] == '\\' || path[1] == '/');
#else
    return path[0] == '/';
#endif
}

std::string join_path(const std::string& lhs, const std::string& rhs)
{
    if (lhs.empty())
    {
        return rhs;
    }
    if (rhs.empty())
    {
        return lhs;
    }
    if (is_absolute_path(rhs))
    {
        return rhs;
    }
    std::string ret = lhs;
    if (!filefunc::is_path_char(ret.back()))
    {
        ret += filefunc::get_path_char();
    }
    ret += rhs;
    return ret;
}

std::vector<std::string> split_path_parts(const std::string& filename)
{
    std::vector<std::string> parts;
    std::string current;
    for (char ch : normalize_path(filename))
    {
        if (filefunc::is_path_char(ch))
        {
            if (!current.empty())
            {
                parts.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += ch;
        }
    }
    if (!current.empty())
    {
        parts.push_back(current);
    }

    std::vector<std::string> normalized;
    for (const auto& part : parts)
    {
        if (part.empty() || part == ".")
        {
            continue;
        }
        if (part == "..")
        {
            if (!normalized.empty() && normalized.back() != "..")
            {
                normalized.pop_back();
                continue;
            }
        }
        normalized.push_back(part);
    }
    return normalized;
}

std::string get_path_root(const std::string& filename)
{
    std::string path = normalize_path(filename);
#ifdef _WIN32
    if (path.size() >= 3 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':' && filefunc::is_path_char(path[2]))
    {
        return path.substr(0, 3);
    }
    if (path.size() >= 2 && filefunc::is_path_char(path[0]) && filefunc::is_path_char(path[1]))
    {
        return std::string(2, '\\');
    }
    return "";
#else
    if (!path.empty() && path[0] == '/')
    {
        return "/";
    }
    return "";
#endif
}

bool equal_path_part(const std::string& a, const std::string& b)
{
#ifdef _WIN32
    if (a.size() != b.size())
    {
        return false;
    }
    for (size_t i = 0; i < a.size(); i++)
    {
        if (std::toupper(static_cast<unsigned char>(a[i])) != std::toupper(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }
    return true;
#else
    return a == b;
#endif
}

void remove_path_recursive(const std::string& path)
{
    if (!filefunc::pathExist(path))
    {
        filefunc::removeFile(path);
        return;
    }
    auto entries = filefunc::getFilesInPath(path, 0, 1, 1);
    for (auto& entry : entries)
    {
        if (filefunc::pathExist(entry))
        {
            remove_path_recursive(entry);
        }
        else
        {
            filefunc::removeFile(entry);
        }
    }
#ifdef _WIN32
    _rmdir(path.c_str());
#else
    rmdir(path.c_str());
#endif
}
}    // namespace

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
    if (!::GetFileAttributesExA(name.c_str(), ::GetFileExInfoStandard, &attrs))
    {
        return false;
    }
    return (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    if (access(name.c_str(), 0) == -1)
    {
        return false;
    }
    struct stat sb;
    stat(name.c_str(), &sb);
    return (sb.st_mode & S_IFDIR) != 0;
#endif
}

size_t filefunc::getFileSize(const std::string& name)
{
    struct stat s;
    if (stat(name.c_str(), &s) != 0)
    {
        return 0;
    }
    return size_t(s.st_size);
}

std::vector<char> filefunc::readFile(const std::string& filename, int length)
{
    std::vector<char> s;
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        return s;
    }
    if (length <= 0)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, 0);
    }
    s.resize(length);
    if (fread(s.data(), 1, length, fp) < size_t(length))
    {
    }
    fclose(fp);
    return s;
}

int filefunc::writeFile(const char* data, int length, const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        fwrite(data, 1, length, fp);
        fflush(fp);
        fclose(fp);
        return length;
    }
    return -1;
}

int filefunc::writeFile(const std::vector<char>& data, const std::string& filename)
{
    return writeFile(data.data(), int(data.size()), filename);
}

std::string filefunc::readFileToString(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, 0);
        std::string str;
        str.resize(length, '\0');
        if (fread((void*)str.c_str(), 1, length, fp) < size_t(length))
        {
        }
        fclose(fp);
        return str;
    }
    return "";
}

int filefunc::writeStringToFile(const std::string& str, const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        int length = int(str.length());
        fwrite(str.c_str(), 1, length, fp);
        fflush(fp);
        fclose(fp);
        return length;
    }
    return -1;
}

bool filefunc::is_path_char(char c)
{
#ifdef _WIN32
    return c == '\\' || c == '/';
#else
    return c == '/';
#endif
}

char filefunc::get_path_char()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

size_t filefunc::getLastPathCharPos(const std::string& filename, int utf8)
{
#ifndef _WIN32
    utf8 = 1;
#endif
    size_t pos = std::string::npos;
    if (utf8 == 0)
    {
        for (int i = 0; i < int(filename.size()); i++)
        {
            if (utf8 == 0 && uint8_t(filename[i]) >= 128)
            {
                i++;
            }
            else if (is_path_char(filename[i]))
            {
                pos = i;
            }
        }
    }
    else
    {
        for (auto i = filename.size(); i--;)
        {
            if (is_path_char(filename[i]))
            {
                pos = i;
                break;
            }
        }
    }
    return pos;
}

size_t filefunc::getLastEftPathCharPos(const std::string& filename, int utf8)
{
#ifndef _WIN32
    utf8 = 1;
#endif
    size_t pos = std::string::npos;
    if (utf8 == 0)
    {
        bool found = false;
        size_t pos1 = std::string::npos;
        for (int i = 0; i < int(filename.size()); i++)
        {
            if (is_path_char(filename[i]))
            {
                if (!found)
                {
                    pos1 = i;
                    found = true;
                }
            }
            else
            {
                pos = pos1;
                found = false;
                if (uint8_t(filename[i]) >= 128)
                {
                    i++;
                }
            }
        }
    }
    else
    {
        bool found_not_path = false;
        int found_not_path_count = 0;
        for (auto i = filename.size(); i--;)
        {
            if (is_path_char(filename[i]))
            {
                found_not_path = false;
            }
            else
            {
                if (!found_not_path)
                {
                    found_not_path_count++;
                    if (found_not_path_count == 2)
                    {
                        return i + 1;
                    }
                }
                found_not_path = true;
            }
        }
    }
    return pos;
}

std::vector<std::string> filefunc::getFilesInPath(const std::string& pathname, int recursive, int include_path, int absolute_path)
{
    if (pathname.empty() || !pathExist(pathname))
    {
        return {};
    }

    if (recursive == 0)
    {
#ifdef _WIN32
        WIN32_FIND_DATAA find_data;
        std::string dir;
        HANDLE h_find = INVALID_HANDLE_VALUE;
        DWORD error = 0;
        std::vector<std::string> files;

        dir = normalize_path(pathname) + "\\*";
        h_find = FindFirstFileA(dir.c_str(), &find_data);

        if (h_find == INVALID_HANDLE_VALUE)
        {
            return files;
        }
        do
        {
            std::string filename = find_data.cFileName;
            if (filename != "." && filename != "..")
            {
                std::string full = join_path(pathname, filename);
                if (include_path != 0 || !pathExist(full))
                {
                    files.push_back(absolute_path ? getAbsolutePath(full) : filename);
                }
            }
        } while (FindNextFileA(h_find, &find_data) != 0);

        error = GetLastError();
        if (error != ERROR_NO_MORE_FILES)
        {
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
            return files;
        }
        while ((ptr = readdir(dir)) != nullptr)
        {
            std::string filename = std::string(ptr->d_name);
            if (filename != "." && filename != "..")
            {
                std::string full = join_path(pathname, filename);
                if (include_path != 0 || !pathExist(full))
                {
                    files.push_back(absolute_path ? getAbsolutePath(full) : filename);
                }
            }
        }
        closedir(dir);
#endif
        return files;
    }
    else
    {
        std::vector<std::string> files, paths;
        paths = getFilesInPath(pathname, 0, 1, 0);
        while (!paths.empty())
        {
            auto p = paths.back();
            paths.pop_back();
            auto full = join_path(pathname, p);
            if (pathExist(full))
            {
                auto new_paths = getFilesInPath(full, 0, 1, 0);
                for (auto& np : new_paths)
                {
                    paths.push_back(join_path(p, np));
                }
                if (include_path)
                {
                    files.push_back(absolute_path ? getAbsolutePath(full) : p);
                }
            }
            else
            {
                files.push_back(absolute_path ? getAbsolutePath(full) : p);
            }
        }
        std::reverse(files.begin(), files.end());
        return files;
    }
}

std::string filefunc::getFileTime(const std::string& filename, const std::string& format)
{
    struct stat s;
    int ret = stat(filename.c_str(), &s);
    if (ret == 0)
    {
        time_t tm_t = s.st_mtime;
        auto filedate = localtime(&tm_t);
        char buf[128] = { 0 };
        strftime(buf, 64, format.c_str(), filedate);
        return buf;
    }
    return "";
}

void filefunc::changePath(const std::string& path)
{
    if (path.empty())
    {
        return;
    }
    if (chdir(path.c_str()) != 0)
    {
        fprintf(stderr, "Failed to change work path %s\n", path.c_str());
    }
}

std::string filefunc::getCurrentPath()
{
    char path_buf[4096] = { 0 };
#ifdef _WIN32
    if (_getcwd(path_buf, sizeof(path_buf)) == nullptr)
#else
    if (getcwd(path_buf, sizeof(path_buf)) == nullptr)
#endif
    {
        return "";
    }
    return path_buf;
}

void filefunc::makePath(const std::string& path)
{
    std::vector<std::string> paths;
    auto p = normalize_path(path) + get_path_char();
    while (true)
    {
        if (pathExist(p) || p == "")
        {
            break;
        }
        paths.push_back(p);
        p = getParentPath(p);
    }
    for (auto it = paths.rbegin(); it != paths.rend(); it++)
    {
        _mkdir(it->c_str());
    }
}

void filefunc::copyFile(const std::string& src, const std::string& dst)
{
    FILE* in = fopen(src.c_str(), "rb");
    if (!in)
    {
        return;
    }
    FILE* out = fopen(dst.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        return;
    }

    char buffer[64 * 1024];
    while (true)
    {
        size_t count = fread(buffer, 1, sizeof(buffer), in);
        if (count > 0)
        {
            fwrite(buffer, 1, count, out);
        }
        if (count < sizeof(buffer))
        {
            break;
        }
    }
    fflush(out);
    fclose(out);
    fclose(in);
}

void filefunc::moveFile(const std::string& src, const std::string& dst)
{
    if (rename(src.c_str(), dst.c_str()) != 0)
    {
        copyFile(src, dst);
        removeFile(src);
    }
}

void filefunc::removeFile(const std::string& filename)
{
    remove(filename.c_str());
}

void filefunc::removePath(const std::string& path)
{
    remove_path_recursive(path);
}

std::string filefunc::getRelativePath(const std::string& filename, const std::string& basepath)
{
    auto abs_file = remove_trailing_path_chars(normalize_path(getAbsolutePath(filename)));
    auto abs_base = remove_trailing_path_chars(normalize_path(getAbsolutePath(basepath)));

    if (fileExist(abs_base))
    {
        abs_base = remove_trailing_path_chars(getParentPath(abs_base, 1));
    }

    if (abs_base.empty())
    {
        return abs_file;
    }

#ifdef _WIN32
    auto file_root = get_path_root(abs_file);
    auto base_root = get_path_root(abs_base);
    if (!equal_path_part(file_root, base_root))
    {
        return abs_file;
    }
#else
    if (get_path_root(abs_file) != get_path_root(abs_base))
    {
        return abs_file;
    }
#endif

    auto file_parts = split_path_parts(abs_file);
    auto base_parts = split_path_parts(abs_base);
    size_t common = 0;
    while (common < file_parts.size() && common < base_parts.size() && equal_path_part(file_parts[common], base_parts[common]))
    {
        common++;
    }

    std::string ret;
    for (size_t i = common; i < base_parts.size(); i++)
    {
        ret += "..";
        ret += get_path_char();
    }
    for (size_t i = common; i < file_parts.size(); i++)
    {
        ret += file_parts[i];
        if (i + 1 < file_parts.size())
        {
            ret += get_path_char();
        }
    }
    if (ret.empty())
    {
        return ".";
    }
    if (ret.size() >= 1 && is_path_char(ret.back()))
    {
        ret.pop_back();
    }
    return ret;
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

std::string filefunc::getFileMainName(const std::string& filename)
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

std::string filefunc::getFileMainNameWithoutPath(const std::string& filename)
{
    return getFilenameWithoutPath(getFileMainName(filename));
}

std::string filefunc::changeFileExt(const std::string& filename, const std::string& ext)
{
    auto e = ext;
    if (!e.empty() && e[0] != '.')
    {
        e = "." + e;
    }
    return getFileMainName(filename) + e;
}

std::string filefunc::getParentPath(const std::string& filename, int utf8)
{
    auto pos_p = getLastEftPathCharPos(filename, utf8);
    if (pos_p != std::string::npos)
    {
        return filename.substr(0, pos_p);
    }
    return "";
}

std::string filefunc::getFilePath(const std::string& filename, int utf8)
{
    return getParentPath(filename, utf8);
}

std::string filefunc::toLegalFilename(const std::string& filename, int allow_path)
{
    std::string f = filename, chars = " *<>?|:";
    if (!allow_path)
    {
        chars += "/\\";
    }
    for (char i = 31; i >= 0; i--)
    {
        chars += i;
    }
    size_t pos = 0;
#ifdef _WIN32
    if (allow_path && f.size() >= 2 && f[1] == ':')
    {
        pos = 2;
    }
#endif
    while ((pos = f.find_first_of(chars, pos)) != std::string::npos)
    {
        f[pos] = '_';
    }
    return f;
}

std::string filefunc::getAbsolutePath(const std::string& filename)
{
    if (filename.empty())
    {
        return getCurrentPath();
    }
#ifdef _WIN32
    DWORD len = GetFullPathNameA(filename.c_str(), 0, nullptr, nullptr);
    if (len == 0)
    {
        return normalize_path(filename);
    }
    std::string full_path(len, '\0');
    DWORD written = GetFullPathNameA(filename.c_str(), len, &full_path[0], nullptr);
    if (written == 0)
    {
        return normalize_path(filename);
    }
    if (!full_path.empty() && full_path.back() == '\0')
    {
        full_path.pop_back();
    }
    return full_path;
#else
    char full_path[4096] = { 0 };
    if (realpath(filename.c_str(), full_path) != nullptr)
    {
        return full_path;
    }
    if (is_absolute_path(filename))
    {
        return filename;
    }
    return join_path(getCurrentPath(), filename);
#endif
}

bool filefunc::compareNature(const std::string& a, const std::string& b)
{
    if (a.empty() && b.empty()) { return false; }
    if (a.empty()) { return true; }
    if (b.empty()) { return false; }
    if (std::isdigit(static_cast<unsigned char>(a[0])) && !std::isdigit(static_cast<unsigned char>(b[0]))) { return true; }
    if (!std::isdigit(static_cast<unsigned char>(a[0])) && std::isdigit(static_cast<unsigned char>(b[0]))) { return false; }
    if (!std::isdigit(static_cast<unsigned char>(a[0])) && !std::isdigit(static_cast<unsigned char>(b[0])))
    {
        if (std::toupper(static_cast<unsigned char>(a[0])) == std::toupper(static_cast<unsigned char>(b[0]))) { return compareNature(a.substr(1), b.substr(1)); }
        {
            return (std::toupper(static_cast<unsigned char>(a[0])) < std::toupper(static_cast<unsigned char>(b[0])));
        }
    }

    std::istringstream issa(a);
    std::istringstream issb(b);
    int ia, ib;
    issa >> ia;
    issb >> ib;
    if (ia != ib) { return ia < ib; }

    std::string anew, bnew;
    std::getline(issa, anew);
    std::getline(issb, bnew);
    return compareNature(anew, bnew);
}