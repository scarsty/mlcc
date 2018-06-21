# Common Library

All libraries are of standard C++ 11 and of cross platforms.

## INIReader

This library contains only one header file.

Read and write ini file. Modified from <https://github.com/benhoyt/inih>.

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
c++ code:

```c++
int main()
{
    INIReader<CaseInsensitivityCompare, NoUnderlineCompare> ini;
    ini.loadFile("example.ini");
    int port = ini.getInt("database", "port", 0);    // port = 143
    int port_ = ini.getInt("database", "port_", 0);    // port_ = 143
    std::string name = ini.getString("owner", "name", "");    //name = Joha Doe
    std::string file = ini.getString("database", "file");    //file = "payroll.dat" (the quotation mark will be kept)
    return 0;
}
```

### Modify an ini file

Use setKey and eraseKey after loaded the file, then use saveFile to write it.

An example:

```c++
int main()
{
    INIReader<CaseInsensitivityCompare, NoUnderlineCompare> ini;
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
This library does not support modifying the comments.

## libconvert

This library contains two files.

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

This library contains only one header file.

A timer.

```c++
Timer t;
// do something...
double elapsed_second = t.getElapsedTime();    //you can check how long the program spent
```

## Random

This library contains only one header file.

An Mersenne twister random number generator.

```c++
Random<double> rand;
rand.set_type(RANDOM_NORMAL);
double d = rand.rand();    //Gaussian(0, 1)
rand.set_type(RANDOM_UNIFORM);
int i = rand.rand_int(100);    //[0, 100)
```

## DynamicLibrary

This library contains two files.

This library is a static class and is of single instance.

Get the C-style function pointer like this:

```c++
void* func = DynamicLibrary::getFunction("xxx.dll", "xxx");
```
The loaded libraries will be unload automatically when the program exits.

## ConsoleControl

This library contains two files.

This library can change the color of the output characters on the console, or change the position of the cursor to format the output.

```c++
ConsoleControl::setColor(CONSOLE_COLOR_LIGHT_RED);    //change the color of printf, fprintf...
ConsoleControl::moveUp(2);    //move up the cursor for 2 lines
```
## File

This library contains two files.

This class can read and write file, and can extract  path, main name or extension from a filename string.

Some functions are similar to libconvert.

```c++
std::string filename = R"(C:\Windows\system32\xxx.exe.1)";
std::string result; 
result = File::getFilePath(filename);    // C:\Windows\system32
result = File::getFileMainname(filename);    // C:\Windows\system32\xxx.exe
result = File::getFilenameWithoutPath(filename);    // xxx.exe.1
result = File::changeFileExt(filename, "dll");    // C:\Windows\system32\xxx.exe.dll
```

On windows, "\\" and "/" are both supported. A mixed style string (such as "C:\Windows\system32/xxx.exe.1") can also be treated correctly. It can treat ANSI string correctly, but for UTF8 string the result may be not right. But Windows cannot open a file with a UTF8 string file name directly, so this problem is not serious.

On Linux and other Unix-like systems, "\\" is not a path pattern, only "/" is effective and it is a single byte character, so the result should always be correct.