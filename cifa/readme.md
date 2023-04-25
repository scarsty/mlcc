# Cifa

## 简介

一个简易类C语法脚本。语法是C的子集，有少量C++风格的扩展。

用于嵌入一些只需要简单计算，且不想引入较复杂的外部库的C++程序。

例如某些情况下，需要在外部配置文件中执行一个简单的C++过程，且希望程序内部的代码可以不修改直接放到配置文件。

项目名就是“词法”。实际上本垃圾一开始理解错了，以为词法分析要生成语法树，就这样吧。

执行模式为直接对语法树求值，并处理分支和循环。

千万不要用它完成过于复杂的任务，正如描述所说，这只是一个非常简单的语法解析器和执行器。如果你想做更复杂的事，请选用Python或者Lua。如果你喜欢C风格的语法，请选用AngelScript或者ChaiScript，但是这两个库都不小，而且在很多Linux发行版上没有预编译包可用。

## 使用方法

### 基本用法

在自己的工程中加入Cifa.h和Cifa.cpp，例程如下：

```c++
#include "Cifa.h"
#include <fstream>
#include <iostream>

using namespace cifa;

int main()
{
    Cifa c1;
    std::ifstream ifs;
    ifs.open("1.c");
    std::string str;
    getline(ifs, str, '\0');
    auto o = c1.run_script(str);
    std::cout << "Cifa value is: " << o.value << "\n";
}
```

其中1.c文件即为脚本内容，一个例子为：

```c++
int sum = 1;
for (int i = 1; i <= 10; i++)
{
    for (int j = 1; j <= 10; j++)
    {
        int x = 0;
        if ((i + j) % 2 == 0)
        {
            x = -1;
        }
        else
            x = 1;
        sum += (i * j) * x;
    }
}
return sum;
```

计算的结果为-24。

若脚本只有一个表达式，则结果就是表达式的求值。若脚本包含多行，则需用return来指定返回值，否则返回值是nan。

### 自定义函数

自定义函数必须可以转化为`std::function<cifa::Object(cifa::ObjectVector&)>`，其中`cifa::ObjectVector`即`std::vector<cifa::Object>`。

#### 方式一
以下自定义3个数学函数（省略了检测越界）：
```c++
using namespace cifa;

Object sin1(ObjectVector& d) { return sin(d[0]); }
Object cos1(ObjectVector& d) { return cos(d[0]); }
Object pow1(ObjectVector& d) { return pow(d[0].value, d[1].value); }

int main()
{
    Cifa c1;
    c1.register_function("sin", sin1);
    c1.register_function("cos", cos1);
    c1.register_function("pow", pow1);
    //....
}
```
这里函数原型写成了xxx1的形式，只是为了避免与cmath中的数学函数同名，造成一些麻烦。

#### 方式二
其实更推荐lambda表达式的形式，例如将上面正弦函数的注册修改为：

```c++
    c1.register_function("sin", [](ObjectVector& d) { return sin(d[0]); });
```
这样也不必再定义sin1这个函数。

此时再运行如下脚本：
```c++
auto pi = 3.1415927;
print(sin(pi / 6));
print(cos(pi / 6));
print(pow(2, 10));
```
输出应是：
```
0.5
0.866025
1024
```
需注意语言已经内置了3个函数：print，to_number和to_string。

### 预定义变量

通过预定义变量可以模拟一些外置函数的效果。下面这个例子中，将pi预先定义好，并将degree视作一个C++送到Cifa的参数:
```c++
    c1.register_parameter("degree", 30);
    c1.register_parameter("pi", 3.14159265358979323846);
```
脚本为：
```c++
print(sin(degree*pi/180));
```
输出应为0.5。

### 语法上的注意事项

- 实际仅有double和string类型，如需自定义类型请自行扩展。
- auto、int、float、double等类型名会被忽略，若想无脑复制可以在C++部分使用auto。
- 未经初始化即出现在赋值号右侧的变量值为nan，相当于强制要求显式初始化。
- 函数调用时，a.func(c)等价于func(a, c)。
- 自加算符不支持++++或----这种写法，请不要瞎折腾。
- 没有switch，do...while，goto，?:。
- 不支持脚本中自定义函数。

## 其他

### 已知问题

- 生成语法树时的检查不太严格，例如if和while后面的条件其实可以不写括号，但是最好要写全（此处若严格处理需要将括号多归约一层，略微影响效率）。
- 报错位置有时不太准确。例如括号被归约后，在语法树上就不再存在，所以会相差一个或多个括号。要解决需要在归约时记录最终位置，比较复杂且意义不是很大，不再处理。

### 有可能会加的

- 变量的括号初始化。
- 若支持自定义函数，则需为每个函数都构造语法树，且必须类似C语言在使用之前声明或定义，暂时搁置。
- 遍历最终语法树可以生成执行码，不再处理。
