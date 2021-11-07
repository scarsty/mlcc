#pragma once
#include <string>
#include <vector>

namespace format1
{

template <typename T>
inline std::string snprint1(const std::string& fmt1, T& t)
{
    char s[128];
    snprintf(s, 128, fmt1.c_str(), t);
    return std::string(s);
}

inline std::string getfmt(const std::string& fmt, const std::string& fmt1)
{
    std::string res = fmt1;
    if (fmt.find_first_of(':') == 0)
    {
        res = "%" + fmt.substr(1);
        if (fmt.back() >= '0' && fmt.back() <= '9')
        {
            res += fmt1.substr(1);
        }
    }
    return res;
}

template <typename T>
inline std::string to_string(const std::string& fmt, const T& t)
{
    return std::to_string(t);
}

template <typename T>
inline std::string to_string(const std::string& fmt, T* t)
{
    return snprint1("%p", t);
}

inline std::string to_string(const std::string& fmt, const std::string& t)
{
    return t;
}

inline std::string to_string(const std::string& fmt, const char* t)
{
    return std::string(t);
}

inline std::string to_string(const std::string& fmt, const double t)
{
    return snprint1(getfmt(fmt, "%g"), t);
}

inline std::string to_string(const std::string& fmt, const float t)
{
    return snprint1(getfmt(fmt, "%g"), t);
}

template <typename T>
inline std::string to_string(const std::string& fmt, const std::vector<T>& t)
{
    if (t.empty())
    {
        return "[]";
    }
    std::string res = "[";
    for (int i = 0; i < t.size() - 1; i++)
    {
        res += to_string(fmt, t[i]) + ", ";
    }
    res += to_string(fmt, t.back()) + "]";
    return res;
}

// array is conflict with pointer
//
//template <typename T, size_t N>
//inline std::string to_string(const std::string& fmt, const T (&t)[N])
//{
//    if (N == 0)
//    {
//        return "[]";
//    }
//    std::string res = "[";
//    for (int i = 0; i < N - 1; i++)
//    {
//        res += to_string(fmt, t[i]) + ", ";
//    }
//    res += to_string(fmt, t[N - 1]) + "]";
//    return res;
//}

inline void format2(size_t pos0, std::string& fmt)
{
}

template <typename T, typename... Args>
inline void format2(size_t pos0, std::string& fmt, const T& t, Args&&... args)
{
    auto pos = fmt.find_first_of('{', pos0);
    if (pos != std::string::npos)
    {
        auto pos1 = fmt.find_first_of('}', pos + 1);
        if (pos1 != std::string::npos)
        {
            pos = fmt.find_last_of('{', pos);
            auto s = to_string(fmt.substr(pos + 1, pos1 - pos - 1), t);
            fmt = fmt.substr(0, pos) + s + fmt.substr(pos1 + 1);
            format2(pos + s.size(), fmt, args...);
        }
    }
}

template <typename... Args>
inline std::string format(const std::string& fmt, Args&&... args)
{
    auto res = fmt;
    format2(0, res, args...);
    return res;
}

template <typename... Args>
inline void print(FILE* fout, const std::string& fmt, Args&&... args)
{
    auto res = format(fmt, args...);
    fprintf(fout, "%s", res.c_str());
}

template <typename... Args>
inline void print(const std::string& fmt, Args&&... args)
{
    print(stdout, fmt, args...);
}

}    // namespace format1