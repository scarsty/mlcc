#include "filefunc.h"
#include <cctype>
#include <chrono>
#include <filesystem>
//#include <format>
#include <fstream>
#include <functional>
#include <sstream>

#ifdef _WIN32
#else
#include "dirent.h"
#ifndef __APPLE__
//#include <sys/io.h>
#endif
#include <sys/uio.h>
#include <unistd.h>
#define _mkdir(p) mkdir(p, S_IRWXU)
#endif

#ifdef __GNUC__
#include <sys/stat.h>
#include <sys/types.h>
#endif

bool filefunc::fileExist(const std::string& name)
{
    return std::filesystem::is_regular_file(name.c_str());
}

bool filefunc::pathExist(const std::string& name)
{
    return std::filesystem::is_directory(name.c_str());
}

std::vector<char> filefunc::readFile(const std::string& filename, int length)
{
    std::ifstream ifs(filename, std::fstream::binary);
    if (!ifs) { return {}; }
    if (length <= 0)
    {
        ifs.seekg(0, std::ios::end);
        length = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
    }
    std::vector<char> buffer(length);
    buffer.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return buffer;
}

int filefunc::writeFile(const char* data, int length, const std::string& filename)
{
    if (filename.empty()) { return 0; }
    std::ofstream ofs(filename, std::fstream::binary);
    ofs.write(data, length);
    return length;
}

int filefunc::writeFile(const std::vector<char>& data, const std::string& filename)
{
    return writeFile(data.data(), data.size(), filename);
}

std::string filefunc::readFileToString(const std::string& filename)
{
    std::ifstream ifs(filename, std::fstream::binary);
    if (!ifs) { return {}; }
    std::string str;
    str.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return str;
}

int filefunc::writeStringToFile(const std::string& str, const std::string& filename)
{
    if (filename.empty()) { return 0; }
    std::ofstream ofs(filename, std::fstream::binary);
    ofs << str;
    return str.size();
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
        //ansi
        for (int i = 0; i < filename.size(); i++)
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
    //avoid mistakes for continued path char
    if (utf8 == 0)
    {
        //ansi
        bool found = false;
        size_t pos1 = std::string::npos;
        for (int i = 0; i < filename.size(); i++)
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

std::vector<std::string> filefunc::getFilesInPath(const std::string& pathname, int recursive /*= 0*/, int include_path /*= 0*/)
{
    if (!pathname.empty() && !std::filesystem::is_directory(pathname.c_str())) { return {}; }
    std::vector<std::string> ret;
    std::filesystem::directory_entry path0;
    if (pathname.empty())
    {
        path0.assign(".");
    }
    else
    {
        path0.assign(pathname);
    }
    std::function<void(std::filesystem::path)> getFiles = [&](std::filesystem::path path)
    {
        for (auto const& dir_entry : std::filesystem::directory_iterator{ path })
        {
            if (std::filesystem::is_directory(dir_entry.path()))
            {
                if (recursive == 1)
                {
                    getFiles(dir_entry.path());
                }
                if (include_path == 1)
                {
                    auto r_path = std::filesystem::relative(dir_entry.path(), path0);
                    ret.push_back(r_path.string());
                }
            }
            else
            {
                auto r_path = std::filesystem::relative(dir_entry.path(), path0);
                ret.push_back(r_path.string());
            }
        }
    };
    getFiles(path0);
    return ret;
}

std::string filefunc::getFileTime(const std::string& filename)
{
    if (!fileExist(filename)) { return ""; }
    auto t = std::filesystem::last_write_time(filename);
    auto t1 = std::chrono::time_point_cast<std::chrono::system_clock::duration>(t - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    auto t2 = std::chrono::system_clock::to_time_t(t1);
    char buf[64] = {};
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t2));
    return buf;
}

void filefunc::changePath(const std::string& path)
{
    if (!path.empty())
    {
        std::filesystem::current_path(path);
    }
}

std::string filefunc::getCurrentPath()
{
    return std::filesystem::current_path().string();
}

void filefunc::makePath(const std::string& path)
{
    if (!path.empty())
    {
        std::filesystem::create_directories(path);
    }
}

void filefunc::copyFile(const std::string& src, const std::string& dst)
{
    std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
}

void filefunc::removeFile(const std::string& filename)
{
    std::filesystem::remove_all(filename);
}

std::string filefunc::getFileExt(const std::string& filename)
{
    auto ext = std::filesystem::path(filename).extension().string();
    if (!ext.empty() && ext[0] == '.')
    {
        ext = ext.substr(1);
    }
    return ext;
}

//find the last point as default, and find the first when mode is 1
std::string filefunc::getFileMainName(const std::string& filename)
{
    auto name = std::filesystem::path(filename).parent_path().string();
    if (!name.empty() && !is_path_char(name.back()))
    {
        name += get_path_char();
    }
    return name + std::filesystem::path(filename).stem().string();
}

std::string filefunc::getFilenameWithoutPath(const std::string& filename)
{
    return std::filesystem::path(filename).filename().string();
}

std::string filefunc::getFileMainNameWithoutPath(const std::string& filename)
{
    return std::filesystem::path(filename).stem().string();
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
    if (filename.size() > 0 && is_path_char(filename.back()))
    {
        return std::filesystem::path(filename).parent_path().parent_path().string();
    }
    else
    {
        return std::filesystem::path(filename).parent_path().string();
    }
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
