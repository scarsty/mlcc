# MLCC Library

Chinese version: [readme.zh-CN.md](readme.zh-CN.md)

MLCC is a set of C++ libraries.

MLCC 是一组 C++ 库。

MLCC means Multi-layer Ceramic Capacitors.

MLCC 的含义是 Multi-layer Ceramic Capacitors，也就是多层陶瓷电容。

All libraries are of standard C++ 11 and of cross platforms.

所有库均基于标准 C++11，并支持跨平台使用。

## Cifa

A simple c-style script, please see [cifa](cifa) for more details. The old develop branch is <https://github.com/scarsty/cifa>, now it has been moved here.

一个简单的 C 风格脚本语言，详情请见 [cifa](cifa)。旧的开发分支原先位于 <https://github.com/scarsty/cifa>，现已迁移到这里。

## INIReader

INIReader.h

Read and write ini file. Modified from <https://github.com/benhoyt/inih>. The writting of it is very quick.

用于读写 ini 文件。基于 <https://github.com/benhoyt/inih> 修改而来，写入速度很快。

### Read an ini file

### 读取 ini 文件

example.ini:

示例文件 example.ini：

```ini
; last modified 1 April 2001 by John Doe
[owner]
name=John Doe
organization=Acme Widgets Inc.

[database]
; use IP address in case network name resolution is not working
server=192.0.2.62
port=143
file="payroll.dat"
```

C++ code:

C++ 代码：

```c++
int main()
{
    INIReaderNoUnderline ini;
    ini.loadFile("example.ini");
    int port = ini.getInt("database", "port", 0);    //port = 143
    int port_ = ini.getInt("database", "port_", 0);    //port_ = 143
    std::string name = ini.getString("owner", "name", "");    //name = Joha Doe
    auto file = ini["database"]["file"].toString();    //file = payroll.dat (the quotation mark will be removed)
    return 0;
}
```

Please note that here we use a subclass to ignore the underline in the keys.

这里使用了一个子类，用来在比较键名时忽略下划线。

### Modify an ini file

### 修改 ini 文件

Use setKey and eraseKey after loading the file, then use saveFile to write results to a file.

加载文件后可通过 setKey 和 eraseKey 进行修改，最后调用 saveFile 写回结果。

An example:

示例：

```c++
int main()
{
    INIReaderNormal ini;
    ini.loadFile("example.ini");
    ini.setKey("", "head", "nothing");
    ini.setKey("owner", "age", "30");
    ini.eraseKey("database", "port");
    ini["owner"].erase("name");
    ini["account"]["password"] = "***";
    ini.saveFile("example1.ini");
    return 0;
}
```

The content of example1.ini after running is:

运行后生成的 example1.ini 内容如下：

```ini
[]
head = nothing
; last modified 1 April 2001 by John Doe
[owner]
organization = Acme Widgets Inc.
age = 30

[database]
; use IP address in case network name resolution is not working
server = 192.0.2.62
file = payroll.dat

[account]
password = ***

```

A space will be added at both sides of "=", and the comments will be maintained.

写出时会在 = 两侧自动补空格，并尽量保留注释。

If the string includes some special charactors, such as ";" and "#" (which are the comment prefix), or line break, please use quote to surround it. Examples:

如果字符串中包含特殊字符，比如 ;、# 这类注释前缀，或者包含换行，请使用引号包裹。示例：

```ini
[sec]
key1 = "cc;dd#l"
key2 = "line1
line2"
```

This library does not support the escape characters or operating the comments.

这个库不支持转义字符，也不支持对注释本身进行编辑。

If a key has been multi-defined, the last value is reserved. **Please note all the multi-defined keys EXCLUDE the first one with the last value will be ERASED when saving!** But the last comment to it will be kept.

如果某个键被重复定义，则只保留最后一个值。请注意：保存时，除了最后一个定义之外，其余同名键都会被删除。但最后一条相关注释会被保留。

You can define how to compare the section and key. Such as:

你也可以自定义 section 和 key 的比较方式，例如：

```c++
struct CaseInsensitivity
{
    std::string operator()(const std::string& l) const
    {
        auto l1 = l;
        std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
        return l1;
    }
};

using INIReaderNormal = INIReader<CaseInsensitivity>;

```

Multi level hierarchy is supported, the sub section should be put inisde two or more square brackets to. Example:

支持多级层级结构。子 section 需要放在两层或更多层方括号中。例如：

```c++
INIReaderNormal ini;
ini["sec0"]["a"] = 1;
ini["sec0"]["sec0_1"]["a"] = 1;
ini["sec1"]["a"] = 1;
```

Note that if you use toString(), toInt() or toDouble() of this type, a new key with empty value will be created if it does not exist.

需要注意，如果你对这个类型调用 toString()、toInt() 或 toDouble()，而对应键不存在，则会自动创建一个值为空的新键。

These C++ code will give an ini file like this:

上述代码会生成如下 ini 文件：

```ini
[sec0]
a=1
[[sec0_1]]
a=1
[sec1]
a=1
```

### About comment

### 关于注释

If you load one file for many times, the comments which do not follow some keys, and blank lines will be repeated at the end of their section. So please do not do this.

如果同一个文件被重复 load 多次，那些没有跟随某个键的注释以及空行，会在其所在 section 的结尾被重复追加。因此不建议这样使用。

## strfunc

strfunc.h, strfunc.cpp

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

vectorToString can convert a std::vector<T> to a std::string, it is too bother to deal with the last splitting char manually.

vectorToString 可以把 std::vector<T> 转成 std::string，省去手动处理最后一个分隔符的麻烦。

It also supplies some encoding converters.

此外还提供了一些编码转换函数。

## Timer

Timer.h

A timer.

一个计时器。

```c++
Timer t;
// do something...
double elapsed_second = t.getElapsedTime();    //you can check how long the program spent
```

## Random

Random.h

An Mersenne twister random number generator.

一个基于 Mersenne Twister 的随机数发生器。

```c++
Random<double> rand;
rand.set_type(RANDOM_NORMAL);
double d = rand.rand();    //Gaussian(0, 1)
rand.set_type(RANDOM_UNIFORM);
int i = rand.rand_int(100);    //[0, 100)
```

## DynamicLibrary

DynamicLibrary.h

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

ConsoleControl.h

This library can change the color of the output characters on the console, or change the position of the cursor to format the output.

这个库可以修改控制台输出字符的颜色，也可以移动光标位置以便格式化输出。

```c++
ConsoleControl::setColor(CONSOLE_COLOR_LIGHT_RED);    //change the color of printf, fprintf...
ConsoleControl::moveUp(2);    //move up the cursor for 2 lines
```

## filefunc

filefunc.h, filefunc.cpp

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

# cmdline

cmdline.h.

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

# runtime_format

A simple runtime format implement, temporary use before C++26 is released.

一个简单的运行时格式化实现，在 C++26 正式可用之前临时使用。

# StrCvt

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

# DrawStringFT

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

# CheckDependency

Only for Windows.

仅支持 Windows。

To check the dependencies of a exe or dll file.

用于检查 exe 或 dll 文件的依赖关系。

The CheckDependency is similar to the data viewed using the command dumpbin /DEPENDENTS.
It can help you get the platform (x64 or x86) of each dll that this exe or dll depends on, as well as the functions exported by each dll, the functions of each dll used by this file, and each problem Dlls (missing functions).

CheckDependency 与命令 dumpbin /DEPENDENTS 所看到的数据相近。
它可以帮助你获取某个 exe 或 dll 依赖的每个 dll 的平台信息，比如 x64 或 x86，以及每个 dll 导出的函数、当前文件实际用到的函数，以及每个有问题的 dll，比如缺失函数的 dll。

An example:

示例：

```c++
CheckDependency dep;
auto problem_dlls = dep.Check("myself.dll");
std::cout << "Problem Dlls : " << std::endl;
for(auto& item : problem_dlls)
{
    std::cout << "(" << item.second.machine.c_str() << ")"
    << item.first << ", lost functions: " << item.second.lost_functions.size()
    << std::endl;
}
std::cout << "Import Table: " << std::endl;
for(auto& item : dep.ImportTable())
{
    std::cout << "(" << item.second.machine.c_str() << ")"
    << item.first << ", used functions: " << item.second.used_functions.size()
    << std::endl;
}
std::cout << "Export Table: " << std::endl;
for(auto& item : dep.ExportTable())
{
    std::cout << item.first << ", export functions: " << item.second.size() << std::endl;
}
```

# FunctionTrait

Check the number of patameters anf the return type of a class member function.

用于检查类成员函数的参数个数以及返回类型。

# FakeJson

A simllified JSON library. It does not support escape characters.

一个简化版 JSON 库，不支持转义字符。

# vramusage

Only for Windows.

仅支持 Windows。

CUDA and HIP supply the apis to get the usage of video memory, but on Windows the result is not right.

CUDA 和 HIP 都提供了获取显存占用的 API，但在 Windows 上结果并不准确。

This can help you to get that correctly.

这个库可以帮助你正确获取显存使用量。

First, get the LUID or PCI bus with cudaGetDeviceProperties / hipGetDeviceProperties, and get the memory usage of it.

首先，通过 cudaGetDeviceProperties 或 hipGetDeviceProperties 获取 LUID 或 PCI 总线信息，再根据这些信息查询对应设备的显存占用。

## 备注

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

# targetlnk

Get the target of a .lnk file.

用于获取 .lnk 文件的目标路径。