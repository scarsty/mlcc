#pragma once

#include <format>
#include <print>

template <typename T>
concept is_printable = requires { std::formatter<std::remove_cvref_t<T>>(); };

template <is_printable... Args>
std::string dyna_format(const std::string_view& fmt, Args&&... args)
{
    return std::vformat(fmt, std::make_format_args(args...));
}

template <is_printable... Args>
void dyna_print(FILE* fout, const std::string_view& fmt, Args&&... args)
{
    std::vprint_nonunicode(fout, fmt, std::make_format_args(args...));
}

template <is_printable... Args>
void dyna_print(const std::string_view& fmt, Args&&... args)
{
    dyna_print(stdout, fmt, std::forward<Args>(args)...);
}

//template <typename T, typename CharT>
//struct std::formatter<T*, CharT>
//{
//    template <typename FormatParseContext>
//    auto parse(FormatParseContext& pc)
//    {
//        return pc.begin();
//    }
//
//    template <typename FormatContext>
//    auto format(const T* t, FormatContext& fc) const
//    {
//        return std::format_to(fc.out(), "{}", (uint64_t)t);
//    }
//};
