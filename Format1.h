#pragma once
#include <string>
#include <vector>
namespace Format1
{

template <typename T>
std::string to_string(const T& t)
{
    return std::to_string(t);
}

std::string to_string(const std::string& t)
{
    return t;
}

std::string to_string(const char* t)
{
    return std::string(t);
}

template <typename T>
std::string to_string(const std::vector<T>& t)
{
    std::string res = "{";
    for (int i = 0; i < t.size() - 1; i++)
    {
        res += to_string(t[i]) + ", ";
    }
    res += to_string(t.back()) + "}";
    return res;
}

template <typename T>
void format2(size_t pos0, std::string& fmt, const T& t)
{
    auto pos = fmt.find_first_of('{', pos0);
    if (pos != std::string::npos)
    {
        auto pos1 = fmt.find_first_of('}', pos + 1);
        if (pos1 != std::string::npos)
        {
            fmt = fmt.substr(0, pos) + to_string(t) + fmt.substr(pos1 + 1);
        }
    }
}

template <typename T, typename... Args>
void format2(size_t pos0, std::string& fmt, const T& t, Args&&... args)
{
    auto pos = fmt.find_first_of('{', pos0);
    if (pos != std::string::npos)
    {
        auto pos1 = fmt.find_first_of('}', pos + 1);
        if (pos1 != std::string::npos)
        {
            auto s = to_string(t);
            fmt = fmt.substr(0, pos) + s + fmt.substr(pos1 + 1);
            format2(pos + s.size(), fmt, args...);
        }
    }
}

template <typename... Args>
std::string format1(const std::string& fmt, Args&&... args)
{
    auto res = fmt;
    format2(0, res, args...);
    return res;
}

}    // namespace Format1