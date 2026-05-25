# MLCC Library

MLCC is a set of C++ libraries.

MLCC 是一组 C++ 库。

MLCC means Multi-layer Ceramic Capacitors.

MLCC 的含义是 Multi-layer Ceramic Capacitors，也就是多层陶瓷电容器。

All libraries are of standard C++ and of cross platforms unless otherwise specified.

所有库基于标准 C++，如无特别说明，均支持跨平台使用。

All libraries are header-only or header + cpp files, you can just put them into your project to use.

均是仅头文件，或者头文件 + cpp 文件的形式，使用时直接把它们放到你的项目里就可以了。

## 目录

### 重要组件
- [Cifa](#cifa)：一个轻量的 C 风格脚本解释器。
- [INIReader](#inireader)：用于读取与写入 ini 配置文件。
- [CheckDependency](#checkdependency)：用于检查 exe/dll 的依赖关系和缺失符号。
- [vramusage](#vramusage)：在 Windows 上查询更准确的显存占用信息。
### 方便使用的小组件
- [strfunc](#strfunc)：提供常见字符串处理与解析函数。
- [filefunc](#filefunc)：提供文件读写与路径处理实用函数。
- [ArchiveFile](#archivefile)：统一封装压缩包读写（7z/zip）。
- [ZipFile](#zipfile)：基于 libzip 的 zip 文件操作封装。
- [ZipFile2](#zipfile2)：自包含 ZIP 读写，内置 inflate/CRC-32，无外部依赖。
- [SimpleCC](#simplecc)：基于词典的简易文本转换工具。
- [PotConv](#potconv)：基于 iconv 的编码转换工具。
- [Timer](#timer)：用于统计代码耗时的计时器。
- [Random](#random)：对随机数生成器的轻量封装。
- [DynamicLibrary](#dynamiclibrary)：动态库加载与函数获取工具。
- [ConsoleControl](#consolecontrol)：控制台颜色与光标控制工具。
- [cmdline](#cmdline)：命令行参数解析工具（含 Windows 细节修正）。
- [StrCvt](#strcvt)：字符串与 UTF8/UTF16/宽字符互转工具。
- [DrawStringFT](#drawstringft)：在图像上绘制文本（含中文）工具。
- [FunctionTrait](#functiontrait)：用于提取成员函数签名信息。
- [targetlnk](#targetlnk)：读取 Windows `.lnk` 快捷方式目标。
- [runtime_format](#runtime_format)：运行时格式化工具（C++26 前置实现）。
- [INIReaderBin](#inireaderbin)：把 ini 元数据与二进制内容打包存储。
- [SimpleBuffer](#simplebuffer)：用于连续内存的简易缓冲区类型。
- [SQLite3Wrapper](#sqlite3wrapper)：SQLite3 的轻量 RAII 封装。
- [versioninfo](#versioninfo)：读取 Windows 可执行文件版本信息。
### 实验组件
- [FakeJson](#fakejson)：不支持转义字符的简化 JSON 组件。
- [CaptureStdio](#capturestdio)：可靠捕获 stdout 输出（支持 tee 模式）。

# Important Components 重要组件

这些是这个项目里面的特色组件，建议优先了解。

## Cifa

Cifa is a simple c-style script, which has been turned into an independent project.

Cifa是一个简单的 C 风格脚本语言，已经转为独立项目。

## INIReader

Read and write ini file. Please see [doc/inireader.md](doc/inireader.md) for detailed usage.

用于读写 ini 文件。完整介绍和示例请见 [doc/inireader.md](doc/inireader.md)。

## CheckDependency

Only for Windows. Please see [doc/checkdependency.md](doc/checkdependency.md) for detailed usage.

仅支持 Windows。完整介绍和 API 参考请见 [doc/checkdependency.md](doc/checkdependency.md)。

## vramusage

Only for Windows.

仅支持 Windows。

CUDA and HIP supply the apis to get the usage of video memory, but on Windows the result is not right.

CUDA 和 HIP 都提供了获取显存占用的 API，但在 Windows 上结果并不准确。

This can help you to get that correctly.

这个库可以帮助你正确获取显存使用量。

First, get the LUID or PCI bus with cudaGetDeviceProperties / hipGetDeviceProperties, and get the memory usage of it.

首先，通过 cudaGetDeviceProperties 或 hipGetDeviceProperties 获取 LUID 或 PCI 总线信息，再根据这些信息查询对应设备的显存占用。

### 备注

此功能用到了未公开用法的 Windows API D3DKMTQueryStatistics 和结构体 D3DKMT_QUERYSTATISTICS。

D3DKMT_QUERYSTATISTICS 是一个以 Union 为主的结构。首先需要设置查询类型和 LUID；查询成功后，Union 的其余部分并不是都有意义。

例如以下查询：

```c++
D3DKMT_QUERYSTATISTICS queryStatistics{};
queryStatistics.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
queryStatistics.AdapterLuid = luid;
if (D3DKMTQueryStatistics(&queryStatistics))
{
    //printf("D3DKMTQueryStatistics failed with %d\n", ret);
    return 1;
}
```

查询之后，只有 queryStatistics.QueryResult.AdapterInformation 是有用的。

在显存查询部分，需要把每个分段的占用量累加起来，才能得到总占用。

# Components for convenience 方便使用的小组件

这些组件的功能比较简单，或者说是一些实用的小工具，可以用来简化开发过程。

## strfunc

example.txt:

示例文件 example.txt：

```
abc_def_;dddd.
```

Some examples:

示例：

```c++
std::string str = readStringFromFile("example.txt");    //str = "abc_def_;dddd."
replaceAllSubStringRef(str, "_", ".");    //str = "abc.def.;dddd."

str = "123,467,222;44";
std::vector<int> numbers = findNumbers<int>(str);    //numbers = {123, 467, 222, 44}, scientific notation is supported
std::vector<std::string> strs = splitString(str, ",;");    //strs = {"123", "467", "222", "44"}
```

"splitString" also supports treating continuous spaces as one pattern.

splitString 也支持把连续空白字符视为同一个分隔模式。

vectorToString can convert a std::vector<T> to a std::string, it is too bother to deal with the last splitting char manually. Now please use std::format.

vectorToString 可以把 std::vector<T> 转成 std::string，省去手动处理最后一个分隔符的麻烦。但是现在建议直接使用 std::format 来实现这个功能了。

## filefunc

This class can read and write file as a vector of any class.

这个类可以把文件按任意类型读取或写入为 vector。

Some functions are very similar to those of "strfunc".

其中有一些函数与 strfunc 中的功能相近。

This class can also extract path, main name or extension from a filename string, examples:

它也可以从文件名字符串中提取路径、主文件名或扩展名，示例：

```c++
std::string filename = R"(C:\Windows\system32\xxx.exe.1)";
std::string result;
result = filefunc::getParentPath(filename);    // C:\Windows\system32
result = filefunc::getFileMainname(filename);    // C:\Windows\system32\xxx.exe
result = filefunc::getFilenameWithoutPath(filename);    // xxx.exe.1
result = filefunc::changeFileExt(filename, "dll");    // C:\Windows\system32\xxx.exe.dll
```

On Windows, "\\" and "/" are both supported. A mixed style string (such as "C:\Windows\system32/xxx.exe.1") can also be treated correctly. It can treat ANSI string correctly, but for UTF8 string the result may be not right. In fact Windows cannot open a file with a UTF8 string file name directly, so this problem is not serious.

在 Windows 上，\\ 和 / 都支持。即使混用路径风格，比如 C:\Windows\system32/xxx.exe.1，也能正确处理。对于 ANSI 字符串结果是正确的，但如果文件名本身是 UTF-8 字符串，结果可能不完全正确。不过 Windows 本身也不能直接用 UTF-8 字符串文件名打开文件，所以这个问题影响不大。

On Linux and other Unix-like systems, "\\" is not a path pattern, only "/" is effective and it is a single byte character in UTF8 coding, so the result should always be correct.

在 Linux 和其他类 Unix 系统上，\\ 不作为路径分隔符，只有 / 有效，并且它在 UTF-8 中也是单字节字符，因此结果通常总是正确的。

## ArchiveFile

A unified archive wrapper based on libarchive, supports common operations for `.7z` and `.zip`.

基于 libarchive 的统一压缩包封装，支持 `.7z` 和 `.zip` 的常见读写操作。

You can create/open archives, add/remove files, list entries, and read file content directly.

支持创建/打开压缩包、增删文件、枚举文件名以及直接读取文件内容。

## ZipFile

A focused zip helper based on libzip.

基于 libzip 的 zip 专用封装。

Supports open/create/read/write/add/remove/list for zip entries with optional password.

支持 zip 条目的打开、创建、读取、写入、添加、删除、列举，并支持可选密码。

## ZipFile2

自包含的 ZIP 文件读写封装，无任何外部依赖。

- 读取支持 STORE（method=0）和 DEFLATE（method=8），可读取标准 ZIP 文件
- 写入使用 STORE 模式（无压缩）
- CRC-32 与 inflate 解压均内置实现（基于 RFC 1951）
- 线程安全（内置 `std::mutex`）
- 支持追加/修改模式（`openWrite` 加载已有条目后再写入）


A simple C++ implementation of Chinese Simplified and Traditional conversion to replace OpenCC.

As OpenCC is too complicated to deploy, this can be a replacement in scenarios where performance requirements are not high.

In fact, it still uses OpenCC's conversion tables, so the conversion results are the same.

用来取代OpenCC进行中文简繁体转换。

因为OpenCC的部署过于复杂，在性能需求不高的场合可以代替。

实际上仍使用OpenCC的转换表，所以转换结果是一样的。

## PotConv

Encoding conversion utilities based on iconv.

基于 iconv 的编码转换工具。

Includes common shortcuts such as CP936/CP950/UTF-8/local conversions.

内置常见快捷转换，如 CP936/CP950/UTF-8/本地编码之间互转。

## Timer

A timer.

一个计时器。

```c++
Timer t;
// do something...
double elapsed_second = t.getElapsedTime();    //you can check how long the program spent
```

## Random

An Mersenne twister random number generator, which is essentially a wrapper of stl.

一个基于 Mersenne Twister 的随机数发生器，本质是调用stl。

```c++
Random<double> rand;
rand.set_type(RANDOM_NORMAL);
double d = rand.rand();    //Gaussian(0, 1)
rand.set_type(RANDOM_UNIFORM);
int i = rand.rand_int(100);    //[0, 100)
```

## DynamicLibrary

This library is a static class and is of single instance.

这是一个静态类，并且以单例方式管理加载的动态库。

Get the C-style function pointer like this:

可以这样获取 C 风格函数指针：

```c++
void* func = DynamicLibrary::getFunction("xxx.dll", "xxx");
```

You have to give the full name of the library include the path.

你需要传入包含完整路径的库文件名。

The loaded libraries will be unload automatically when the program exits.

程序退出时，已加载的库会自动卸载。

## ConsoleControl

This library can change the color of the output characters on the console, or change the position of the cursor to format the output.

这个库可以修改控制台输出字符的颜色，也可以移动光标位置以便格式化输出。

```c++
ConsoleControl::setColor(CONSOLE_COLOR_LIGHT_RED);    //change the color of printf, fprintf...
ConsoleControl::moveUp(2);    //move up the cursor for 2 lines
```

## cmdline

Modified from <https://github.com/tanakh/cmdline>. Please read the instruction on the original project.

基于 <https://github.com/tanakh/cmdline> 修改而来。其基本用法请参考原项目说明。

A bug when parsing a full command line string has been corrected.

这里修复了一个“解析完整命令行字符串”时的 bug。

You'd better to use it like this:

在 Windows 上更建议这样使用：

```c++

#ifdef _WIN32    // or _MSC_VER, as you wish
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
...
#ifdef _WIN32
    cmd.parse_check(GetCommandLineA());
#else
    cmd.parse_check(argc, argv);
#endif
...
}
```

or a command line mixing backslash and quote cannot be correctly parsed on Windows. For an example:

否则在 Windows 上，包含反斜杠和引号混合的命令行参数可能无法被正确解析。例如：

```shell
something.exe --path "C:\Windows\system32\" --other-option values
```

In this case, "argc" and "argv" in the program are NOT right with CMD, but are right with Power Shell, is it a bug of Windows?

在这种情况下，使用 CMD 启动时，程序中的 argc 和 argv 结果并不正确；而在 PowerShell 中却是正确的。这是否算 Windows 的一个 bug，就见仁见智了。

## StrCvt

Some practical functions involving string coding, wide characters, and multi-byte characters.

提供一些与字符串编码、宽字符、多字节字符相关的实用函数。

Requires >= C++11 and < C++26.

要求 C++ 版本 >= C++11 且 < C++26。

```c++
std::string strfunc::CvtStringToUTF8(const std::string& localstr); //windows only
std::string strfunc::CvtUTF8ToLocal(const std::string& utf8str); //windows only

std::string strfunc::CvtStringToUTF8(const char16_t& src);
std::string strfunc::CvtStringToUTF8(const std::u16string& src);
std::string strfunc::CvtStringToUTF8(const wchar_t* start, std::uint64_t len);
std::string strfunc::CvtStringToUTF8(const std::wstring& str);
std::u16string strfunc::CvtStringToUTF16(const std::string& src);
std::u16string strfunc::CvtStringToUTF16(const char* start, int len);
std::wstring strfunc::CvtStringToWString(const std::string& src);
std::wstring strfunc::CvtStringToWString(const char* start, uint64_t len);

```

## DrawStringFT

A C++ warp for freetype and opencv Mat. It can write charactors (such as Chinese) on an image.

一个针对 freetype 和 opencv Mat 的 C++ 封装，可以把字符，比如中文，写到图像上。

Usage:

用法：

```c++
DrawStringFT ft;
ft.openFont(R"(C:\Windows\Fonts\simkai.ttf)", 30);    //Font name and size
cv::Mat m = cv::imread(R"(1.png)");
// parameters:
// text, image, x, y, color (BGR usually), fusion (0 or 1), back ground (0 means no back, and 255 means black)， coding of the string
ft.drawString((char*)u8"黄埃散漫风萧索，云栈萦纡登剑阁", m, 50, 100, { 0x87, 0x89, 0xf0 }, 1, 192, "utf-8");
```

## FunctionTrait

Check the number of patameters anf the return type of a class member function.

用于检查类成员函数的参数个数以及返回类型。

## targetlnk

Get the target of a .lnk file.

用于获取 .lnk 文件的目标路径。

## runtime_format

A simple runtime format implement, it should be a part of C++26, temporary use before C++26 is released.

一个简单的运行时格式化实现，它应该会是C++26的一部分，在 C++26 正式可用之前临时使用。

## INIReaderBin

Binary container format for INI key-values + binary payload.

用于 INI 键值 + 二进制内容的封装格式。

Useful when text metadata and binary blocks need to be stored in one file.

适用于需要把文本元数据和二进制块放在同一个文件中的场景。

## SimpleBuffer

A tiny buffer type for plain memory usage.

一个轻量级缓冲区类型，用于简单内存管理场景。

It can be used as a simpler alternative to `std::vector` when only contiguous storage is needed.

当只需要连续内存而不需要完整容器能力时，可作为 `std::vector` 的简化替代。

## SQLite3Wrapper

Lightweight RAII wrapper for sqlite3.

sqlite3 的轻量 RAII 封装。

Provides helpers for connection management, execute/query, prepared statements, parameter binding and blob read/write.

提供连接管理、执行/查询、预处理语句、参数绑定以及 blob 读写等常用能力。

## versioninfo

Windows-only helpers to read file version and string metadata from PE version resources.

仅限 Windows，用于从 PE 版本资源读取文件版本号和字符串元数据。

# 实验组件

只是一些正在测试中的组件，可能不太稳定，或者功能不完善。

## FakeJson

A simllified JSON library. It does not support escape characters.

一个简化版 JSON 库，不支持转义字符。

## CaptureStdio

This is written by AI.

这个其实是AI写的。

Reliably capture stdout output from function calls (e.g. DLL). Please see [doc/capturestdio.md](doc/capturestdio.md) for more details.

可靠地捕获函数调用（如 DLL）中的 stdout 输出。使用 fd 级重定向实现，支持 tee 模式（实时控制台显示 + 字符串捕获），保留控制台色彩。详见 [doc/capturestdio.md](doc/capturestdio.md)。