#pragma once

#include <format>
#include <print>

namespace std
{

struct runtime_format
{
    string_view fmt;
};

template <typename T>
concept is_printable = requires { std::formatter<std::remove_cvref_t<T>>(); };

template <is_printable... Args>
std::string format(const runtime_format& rfmt, Args&&... args)
{
    return std::vformat(rfmt.fmt, std::make_format_args(args...));
}

template <is_printable... Args>
void print(FILE* fout, const runtime_format& rfmt, Args&&... args)
{
    std::vprint_nonunicode(fout, rfmt.fmt, std::make_format_args(args...));
}

template <is_printable... Args>
void print(const runtime_format& rfmt, Args&&... args)
{
    print(stdout, rfmt, std::forward<Args>(args)...);
}
}    //namespace std
