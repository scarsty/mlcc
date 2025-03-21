# MLCC Library

MLCC is a set of C++ libraries.

MLCC means Multi-layer Ceramic Capacitors.

All libraries are of standard C++ 11 and of cross platforms.

## Cifa

A simple c-style script, please see [cifa](cifa) for more details. The old develop branch is <https://github.com/scarsty/cifa>, now it has been moved here.

## INIReader

INIReader.h

Read and write ini file. Modified from <https://github.com/benhoyt/inih>. The writting of it is very quick.

### Read an ini file

example.ini:

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

### Modify an ini file

Use setKey and eraseKey after loading the file, then use saveFile to write results to a file.

An example:

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

If the string includes some special charactors, such as ";" and "#" (which are the comment prefix), or line break, please use quote to surround it. Examples:

```ini
[sec]
key1 = "cc;dd#l"
key2 = "line1
line2"
```

This library does not support the escape characters or operating the comments.

If a key has been multi-defined, the last value is reserved. **Please note all the multi-defined keys EXCLUDE the first one with the last value will be ERASED when saving!** But the last comment to it will be kept.

You can define how to compare the section and key. Such as:

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

```c++
INIReaderNormal ini;
ini["sec0"]["a"] = 1;
ini["sec0"]["sec0_1"]["a"] = 1;
ini["sec1"]["a"] = 1;
```
Note that if you use toString(), toInt() or toDouble() of this type, a new key with empty value will be created if it does not exist.

These C++ code will give an ini file like this:

```ini
[sec0]
a=1
[[sec0_1]]
a=1
[sec1]
a=1
```

### About comment

If you load one file for many times, the comments will be repeated at the end of their section. So please do not do this.

## strfunc

strfunc.h, strfunc.cpp

example.txt:

```
abc_def_;dddd.
```

Some examples:

```c++
std::string str = readStringFromFile("example.txt");    //str = "abc_def_;dddd."
replaceAllSubStringRef(str, "_", ".");    //str = "abc.def.;dddd."

str = "123,467,222;44";
std::vector<int> numbers = findNumbers<int>(str);    //numbers = {123, 467, 222, 44}, scientific notation is supported 
std::vector<std::string> strs = splitString(str, ",;");    //strs = {"123", "467", "222", "44"}
```

"splitString" also supports treating continuous spaces as one pattern.

vectorToString can convert a std::vector<T> to a std::string, it is too bother to deal with the last splitting char manually.

It also supplies some encoding converters.

## Timer

Timer.h

A timer.

```c++
Timer t;
// do something...
double elapsed_second = t.getElapsedTime();    //you can check how long the program spent
```

## Random

Random.h

An Mersenne twister random number generator.

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

Get the C-style function pointer like this:

```c++
void* func = DynamicLibrary::getFunction("xxx.dll", "xxx");
```

You have to give the full name of the library include the path.

The loaded libraries will be unload automatically when the program exits.

## ConsoleControl

ConsoleControl.h

This library can change the color of the output characters on the console, or change the position of the cursor to format the output.

```c++
ConsoleControl::setColor(CONSOLE_COLOR_LIGHT_RED);    //change the color of printf, fprintf...
ConsoleControl::moveUp(2);    //move up the cursor for 2 lines
```
## filefunc

filefunc.h, filefunc.cpp

This class can read and write file as a vector of any class.

Some functions are very similar to those of "strfunc".

This class can also extract  path, main name or extension from a filename string, examples:

```c++
std::string filename = R"(C:\Windows\system32\xxx.exe.1)";
std::string result; 
result = filefunc::getParentPath(filename);    // C:\Windows\system32
result = filefunc::getFileMainname(filename);    // C:\Windows\system32\xxx.exe
result = filefunc::getFilenameWithoutPath(filename);    // xxx.exe.1
result = filefunc::changeFileExt(filename, "dll");    // C:\Windows\system32\xxx.exe.dll
```

On Windows, "\\" and "/" are both supported. A mixed style string (such as "C:\Windows\system32/xxx.exe.1") can also be treated correctly. It can treat ANSI string correctly, but for UTF8 string the result may be not right. In fact Windows cannot open a file with a UTF8 string file name directly, so this problem is not serious.

On Linux and other Unix-like systems, "\\" is not a path pattern, only "/" is effective and it is a single byte character in UTF8 coding, so the result should always be correct.

# cmdline

cmdline.h.

Modified from <https://github.com/tanakh/cmdline>. Please read the instruction on the original project.

A bug when parsing a full command line string has been corrected.

You'd better to use it like this:

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
```shell
something.exe --path "C:\Windows\system32\" --other-option values
```
In this case, "argc" and "argv" in the program are NOT right with CMD, but are right with Power Shell, is it a bug of Windows?

# fmt1

A simple substitute of std::format. 

If you cannot stand the neglect of Clang and GCC, maybe you can try it.

It also provide the formatter of vector and map to use is C++20.

It has been removed due to the supporting of the mainstream compilers.

# PotConv

A C++ warp for iconv.

# StrCvt

Some practical functions involving string coding, wide characters, and multi-byte characters.
Requires >= C++11 and < C++26.

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

# CheckDependency

Only for Windows.

To check the dependencies of a exe or dll file. 

The CheckDependency is similar to the data viewed using the command `dumpbin /DEPENDENTS`.
It can help you get the platform (x64 or x86) of each dll that this exe or dll depends on, as well as the functions exported by each dll, the functions of each dll used by this file, and each problem Dlls (missing functions).

An example:

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

# FakeJson

A simllified JSON library. It does not support escape characters.

# vramusage

Only for Windows.

CUDA and HIP supply the apis to get the usage of video memory, but on Windows the result is not right.

This can help you to get that correctly. 

First, get the LUID or PCI bus with cudaGetDeviceProperties / hipGetDeviceProperties, and get the memory usage of it.

## 备注

此功能用到了未公开用法的Windows API`D3DKMTQueryStatistics`和结构体`D3DKMT_QUERYSTATISTICS`。

`D3DKMT_QUERYSTATISTICS`是一个以Union为主的结构，首先需要赋值查询内容和LUID，查询成功之后，Union的其余部分是没有用的。

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

查询之后， 只有`queryStatistics.QueryResult.AdapterInformation`是有用的。

在显存查询的部分，需要把每段的占用加起来得到总的占用。

# targetlnk

Get the target of a .lnk file.

