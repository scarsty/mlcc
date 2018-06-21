# Common Library

## INIReader

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

Sometimes, if you want to ignore the underlines in the key string, you should declare a new class like this:

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
This library does not support to modify the comments.

## libconvert

常用字串函数。

Timer

计时。

Random

随机数。

DynamicLibrary

载入动态库。

ConsoleControl

终端控制。改变字符颜色，控制光标位置等。

File

文件相关，读取文件，文件名处理等。