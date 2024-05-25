#pragma once
#include <cctype>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>

namespace fmt1
{

inline std::string getfmt(const std::string& fmt, const std::string& fmt_s)
{
    std::string res = fmt_s;
    if (fmt.find_first_of(':') == 0)
    {
        res = "%" + fmt.substr(1);
        if (!isalpha(fmt.back()))
        {
            res += fmt_s.substr(1);
        }
    }
    return res;
}

template <typename T>
inline std::string sprintf1(const std::string& fmt, const T& t)
{
    int len = snprintf(nullptr, 0, fmt.c_str(), t);
    std::string res(len + 1, '\0');
    snprintf((char*)res.data(), len + 1, fmt.c_str(), t);    //C++11中尾部含\0的空间，可以不处理此处
    res.pop_back();
    return res;
}

template <typename T>
inline std::string sprintf2(const std::string& fmt, const std::string& fmt_s, const T& t)
{
    return sprintf1(getfmt(fmt, fmt_s), t);
}

template <typename T>
inline std::string to_string(const std::string& fmt, const T& t)
{
    return typeid(T).name();    //unknown type
}

template <typename T>
inline std::string to_string(const std::string& fmt, T* t)
{
    return sprintf2(fmt, "%p", t);
}

inline std::string to_string(const std::string& fmt, const char* t)
{
    return sprintf2(fmt, "%s", t);
}

inline std::string to_string(const std::string& fmt, const char t)
{
    return sprintf2(fmt, "%c", t);
}

inline std::string to_string(const std::string& fmt, const signed char t)
{
    return sprintf2(fmt, "%hhd", t);
}

inline std::string to_string(const std::string& fmt, const unsigned char t)
{
    return sprintf2(fmt, "%hhu", t);
}

inline std::string to_string(const std::string& fmt, const short t)
{
    return sprintf2(fmt, "%hd", t);
}

inline std::string to_string(const std::string& fmt, const unsigned short t)
{
    return sprintf2(fmt, "%hu", t);
}

inline std::string to_string(const std::string& fmt, const int t)
{
    return sprintf2(fmt, "%d", t);
}

inline std::string to_string(const std::string& fmt, const unsigned t)
{
    return sprintf2(fmt, "%u", t);
}

inline std::string to_string(const std::string& fmt, const long t)
{
    return sprintf2(fmt, "%ld", t);
}

inline std::string to_string(const std::string& fmt, const unsigned long t)
{
    return sprintf2(fmt, "%lu", t);
}

inline std::string to_string(const std::string& fmt, const long long t)
{
    return sprintf2(fmt, "%lld", t);
}

inline std::string to_string(const std::string& fmt, const unsigned long long t)
{
    return sprintf2(fmt, "%llu", t);
}

inline std::string to_string(const std::string& fmt, const double t)
{
    return sprintf2(fmt, "%g", t);
}

inline std::string to_string(const std::string& fmt, const float t)
{
    return sprintf2(fmt, "%g", t);
}

inline std::string to_string(const std::string& fmt, const bool t)
{
    return sprintf2(fmt, "%d", t);
}

inline std::string to_string(const std::string& fmt, const std::string& t)
{
    return sprintf2(fmt, "%s", t.c_str());
}

template <typename T>
inline std::string to_string(const std::string& fmt, const std::vector<T>& t)
{
    if (t.empty())
    {
        return "[]";
    }
    std::string res = "[";
    for (auto& i : t)
    {
        res += to_string(fmt, i) + ", ";
    }
    res.pop_back();
    res.back() = ']';
    return res;
}

template <typename T1, typename T2>
inline std::string to_string(const std::string& fmt, const std::map<T1, T2>& t)
{
    if (t.empty())
    {
        return "[]";
    }
    std::string res = "[";
    for (auto& i : t)
    {
        res += to_string(fmt, i.first) + ": " + to_string(fmt, i.second) + ", ";
    }
    res.pop_back();
    res.back() = ']';
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

}    // namespace fmt1
