#pragma once

#include <format>
#include <string>
#include <string_view>
#include <cstdio>
#include <utility>

namespace std
{

struct runtime_format {
    std::string_view fmt;
    constexpr runtime_format(std::string_view s) : fmt(s) {}
};

// 检查类型T是否可由 std::formatter 格式化
template <typename T>
concept is_printable = requires { std::formatter<std::remove_cvref_t<T>, char>{}; };

// 使用 std::make_tuple 与 std::apply 来捕获参数，使得它们在传入 std::make_format_args 时均为 lvalue
template <is_printable... Args>
inline std::string format(const runtime_format& rfmt, Args&&... args) {
    // 如果是 rvalue 则移动，lvalue 则复制
    auto tup = std::make_tuple(std::forward<Args>(args)...);
    // 用std::apply解包tuple，将元素作为 lvalue 传入 std::make_format_args
    return std::apply(
        [&](auto&... unwrapped) {
            return std::vformat(rfmt.fmt, std::make_format_args(unwrapped...));
        },
        tup
    );
}

// 与上面format()类似的捕获参数为了完美转发，只是这里是输出到FILE*
template <is_printable... Args>
inline void print(FILE* fout, const runtime_format& rfmt, Args&&... args) {
    // 如果是 rvalue 则移动，lvalue 则复制
    auto tup = std::make_tuple(std::forward<Args>(args)...);
    // 用std::apply解包tuple
    std::string formatted = std::apply(
        [&](auto&... unwrapped) {
            return std::vformat(rfmt.fmt, std::make_format_args(unwrapped...));
        },
        tup
    );
    std::fputs(formatted.c_str(), fout);
}

// 实现个默认版本(to stdout)，方便外面调用
template <is_printable... Args>
inline void print(const runtime_format& rfmt, Args&&... args) {
    print(stdout, rfmt, std::forward<Args>(args)...);
}

}