# INIReader

INIReader.h

Read and write ini file. Modified from <https://github.com/benhoyt/inih>. The writting of it is very quick.

用于读写 ini 文件。基于 <https://github.com/benhoyt/inih> 修改而来，写入速度很快。

## Read an ini file

## 读取 ini 文件

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

## Modify an ini file

## 修改 ini 文件

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

## About comment

## 关于注释

If you load one file for many times, the comments which do not follow some keys, and blank lines will be repeated at the end of their section. So please do not do this.

如果同一个文件被重复 load 多次，那些没有跟随某个键的注释以及空行，会在其所在 section 的结尾被重复追加。因此不建议这样使用。
