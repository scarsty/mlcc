# Common Libraries

These libraries are parts of Will.

All libraries are of standard C++ 11 and of cross platforms.

## INIReader

INIReader.h

Read and write ini file. Modified from <https://github.com/benhoyt/inih>.

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
    INIReader<CaseInsensitivityCompare, NoUnderlineCompare> ini;
    ini.loadFile("example.ini");
    int port = ini.getInt("database", "port", 0);    //port = 143
    int port_ = ini.getInt("database", "port_", 0);    //port_ = 143
    std::string name = ini.getString("owner", "name", "");    //name = Joha Doe
    std::string file = ini.getString("database", "file");    //file = "payroll.dat" (the quotation mark will be kept)
    return 0;
}
```

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
    ini.setKey("account", "password", "***");
    ini.saveFile("example1.ini");
    return 0;
}
```
The content of example1.ini after running is:

```ini
head = nothing
; last modified 1 April 2001 by John Doe
[owner]
name=John Doe
organization=Acme Widgets Inc.
age = 30

[database]
; use IP address in case network name resolution is not working
server=192.0.2.62     
file="payroll.dat"

[account]
password = ***

```
This library does not support operating the comments.

If a key has been multi-defined, the last value should be taken. **Please note all the multi-defined lines EXCLUDE the last one will be ERASED when save!**

### Template Class

You have to declare a new class which describes how to deal the key string at the beginning. For an example:

```c++
struct CaseInsensitivityCompare
{
    bool operator()(const std::string& l, const std::string& r) const
    {
        auto l1 = l;
        auto r1 = r;
        std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
        std::transform(r1.begin(), r1.end(), r1.begin(), ::tolower);
        return l1 < r1;
    }
};
```

Then declare the ini object:

```c++
INIReader<CaseInsensitivityCompare, CaseInsensitivityCompare> ini;
```

Sometimes, if you want to ignore the underlines in the key string, you should declare a new class like this first:

```c++
struct NoUnderlineCompare
{
    bool operator()(const std::string& l, const std::string& r) const
    {
        auto l1 = l;
        auto r1 = r;
        auto replaceAllString = [](std::string& s, const std::string& oldstring, const std::string& newstring)
        {
            int pos = s.find(oldstring);
            while (pos >= 0)
            {
                s.erase(pos, oldstring.length());
                s.insert(pos, newstring);
                pos = s.find(oldstring, pos + newstring.length());
            }
        };
        replaceAllString(l1, "_", "");
        replaceAllString(r1, "_", "");
        std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
        std::transform(r1.begin(), r1.end(), r1.begin(), ::tolower);
        return l1 < r1;
    }
};
```

Then declare the ini object like this:

```c++
INIReader<CaseInsensitivityCompare, NoUnderlineCompare> ini;
```

A case insensitivity type "INIReaderNormal" has been defined in the head file, you can use it directly for convenience:

```c++
typedef INIReader<CaseInsensitivityCompare, CaseInsensitivityCompare> INIReaderNormal;
```

## convert

convert.h, convert.cpp

example.txt:

```
abc_def_;dddd.
```

Some examples:

```c++
std::string str = readStringFromFile("example.txt");    //str = "abc_def_;dddd."
replaceAllString(str, "_", ".");    //str = "abc.def.;dddd."

str = "123,467,222;44";
std::vector<int> numbers = findNumbers<int>(str);    //numbers = {123, 467, 222, 44}, scientific notation is supported 
std::vector<std::string> strs = splitString(str, ",;");    //strs = {"123", "467", "222", "44"}
```

"splitString" also supports treating continuous spaces as one pattern.

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
## File

File.h, File.cpp

This class can read and write file as a vector of any class.

Some functions are very similar to those of libconvert.

This class can also extract  path, main name or extension from a filename string, examples:

```c++
std::string filename = R"(C:\Windows\system32\xxx.exe.1)";
std::string result; 
result = File::getFilePath(filename);    // C:\Windows\system32
result = File::getFileMainname(filename);    // C:\Windows\system32\xxx.exe
result = File::getFilenameWithoutPath(filename);    // xxx.exe.1
result = File::changeFileExt(filename, "dll");    // C:\Windows\system32\xxx.exe.dll
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

