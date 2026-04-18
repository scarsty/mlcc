# CaptureStdio

可靠地捕获函数（如 DLL 调用）中的 stdout 输出。

## 原理

使用 fd 级重定向（`pipe` + `dup2` + 读取器线程）：

```
            ┌────────────┐
stdout ───► │ pipe write  │
  (fd 1)    └─────┬──────┘
                  │ pipe
            ┌─────▼──────┐
            │ pipe read   │ ◄── reader thread
            └─────┬──────┘
                  │
          ┌───────┴────────┐
          │                │
     append to        _write to
     captured         saved_fd
      string          (console)
```

1. `dup(1)` 保存原始 stdout fd（指向控制台）
2. `dup2(pipe_write, 1)` 将 stdout 重定向到管道写入端
3. 读取器线程从管道读取数据，同时：
   - 累积到字符串（用于日志记录）
   - 可选地写回原始控制台 fd（实时显示，保留色彩）
4. 函数执行完毕后恢复 stdout，读取器线程排空剩余数据

## 色彩保留

Windows 控制台颜色通过 `SetConsoleTextAttribute` 设置在控制台 HANDLE 上，与 C 运行时的 fd 重定向（`_dup2`）无关。读取器线程将数据写回原始控制台 fd 时，当前的文本属性仍然生效。

因此，只需在 DLL 调用前设置控制台颜色，tee 模式即可保留该颜色。

## 用法

### 基本用法

```c++
#include "CaptureStdio.h"

// 捕获并同时在控制台显示（tee 模式）
std::string output = CaptureStdio::capture([&]() {
    some_dll_function();
});

// 仅捕获，不在控制台显示
std::string output = CaptureStdio::capture([&]() {
    some_dll_function();
}, false);
```

### 结合控制台颜色

```c++
#include "CaptureStdio.h"
#include "ConsoleControl.h"

ConsoleControl::setColor(8);    // 设置灰色
std::string output = CaptureStdio::capture([&]() {
    dll_detect_image(image);    // DLL 的 printf 输出会以灰色显示在控制台
}, CaptureStdio::isConsole());  // 仅在控制台环境下 tee
ConsoleControl::resetColor();

// output 中包含 DLL 的全部 stdout 输出，可写入日志
```

### 结合日志系统

```c++
std::string captured = CaptureStdio::capture([&]() {
    dll_function();
}, CaptureStdio::isConsole());

// 写入日志文件（不再打印到控制台，避免重复）
for (auto& line : strfunc::splitString(captured, "\n"))
{
    if (!line.empty())
        FMTLOG(fmtlog::INF, "{}", line);
}
```

## API

### `CaptureStdio::capture(func, tee)`

| 参数 | 类型 | 说明 |
|------|------|------|
| `func` | `std::function<void()>` | 要执行的函数 |
| `tee` | `bool`（默认 `true`） | 是否同时在控制台实时显示 |
| 返回值 | `std::string` | 捕获的 stdout 内容 |

线程安全：内部使用全局互斥锁，多线程调用会串行化。

### `CaptureStdio::isConsole()`

| 返回值 | 类型 | 说明 |
|--------|------|------|
| 是否控制台 | `bool` | stdout 是否连接到终端 |

## 注意事项

- 如果管道创建失败，会直接执行函数并返回空字符串（降级处理）
- 使用 `_O_BINARY` 模式（Windows），避免 CR/LF 转换问题
- 捕获期间，函数内的 `printf`、`std::cout`、`puts` 等均会被截获
- 捕获不影响 stderr
