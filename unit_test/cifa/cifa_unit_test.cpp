#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include "../../Cifa.h"

using namespace cifa;

bool register_function_test() {
    Cifa c1;
    c1.register_function("sin", [](ObjectVector& d) { return sin(d[0]); });

    std::string script_code = R"(
    double PI = 3.141592653589793238462643383279;
    double return_val = 0;
    return_val += sin(0);
    return_val += sin(PI * 0.5);
    return_val += sin(PI);
    return return_val;
    )";

    auto o = c1.run_script(script_code);
    if (o.hasValue() && o.isNumber() && o.isType<double>()) {
        return (std::fabs(o.ref<double>() - 1.0) <= std::numeric_limits<double>::epsilon());
    } else {
        return false;
    }
}

bool loop_math_test()
{
    Cifa c1;
    std::string script_code = R"(
    int i;
    double sum = 0.0, product = 1.0, division = 100.0, difference = 50.0;
    double total_result = 0.0;
    for (i = 1; i <= 5; i++) {
        sum += i;
    }
    while (i <= 6) {
        product *= i;
        i++;
    }
    do {
        difference -= i;
        i++;
    } while (difference > 0);
    division /= 5;
    total_result = sum + product + division + difference;
    return total_result;
    )";

    auto o = c1.run_script(script_code);
    if (o.hasValue() && o.isNumber() && o.isType<double>()) {
        return (std::fabs(o.ref<double>() - 34) <= std::numeric_limits<double>::epsilon());
    } else {
        return false;
    }
}

bool ternary_operator_test()
{   // 测试嵌套三目运算
    Cifa c;
    std::string script = R"(
        int a = 1, b = 0;
        return a > b ? (b > a ? 10 : 20) : 30;
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.toInt() == 20;
}

bool switch_case_test()
{   // Switch-Case 完备性测试
    Cifa c;
    std::string script = R"(
        int x = 2;
        int res = 0;
        switch(x) {
            case 1: res = 10; break;
            case 2: res = 20; // 故意不写break看看？
            case 3: res = 30; break;
            default: res = 40;
        }
        return res;
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.toInt() == 30;
}

bool recursion_test()
{   // 递归函数测试
    Cifa c;
    std::string script = R"(
        double factorial(double n) {
            if (n <= 1) return 1;
            return n * factorial(n - 1);
        }
        return factorial(5);
    )";
    auto o = c.run_script(script);
    return o.hasValue() && std::fabs(o.toDouble() - 120.0) < 1e-9;
}

bool string_operation_test()
{   // 字符串操作与拼接测试
    Cifa c;
    std::string script = R"(
        string s1 = "Hello ";
        string s2 = "World";
        return s1 + s2;
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.isType<std::string>() && o.toString() == "Hello World";
}

bool bitwise_operator_test()
{   // 位运算测试
    Cifa c;
    std::string script = R"(
        int a = 5;      // 0101
        int b = 3;      // 0011
        int res1 = a & b;  // 0001 (1)
        int res2 = a | b;  // 0111 (7)
        int res3 = a ^ b;  // 0110 (6)
        int res4 = a << 1; // 1010 (10)
        return res1 + res2 + res3 + res4; // 1 + 7 + 6 + 10 = 24
    )";
    auto o = c.run_script(script);
    return o.toInt() == 24;
}

bool scope_shadowing_test()
{   // 变量作用域遮蔽测试
    Cifa c;
    std::string script = R"(
        int x = 10;
        {
            int x = 20;
            if (x == 20) {
                int x = 30;
            }
        }
        return x;
    )";
    auto o = c.run_script(script);
    return o.toInt() == 10; // 外部作用域不应受内部干扰
}

bool complex_math_priority_test()
{   // 复杂算术优先级测试
    Cifa c;
    std::string script = R"(
        return 2 + 3 * 4 / (1 + 1) - 5 % 2; // 2 + 12 / 2 - 1 = 2 + 6 - 1 = 7
    )";
    auto o = c.run_script(script);
    return o.toInt() == 7;
}

bool array_access_test()
{   // 数组/集合模拟测试 (假设Cifa支持类似[]的操作)
    Cifa c;
    std::string script = R"(
        //int arr[3];
        arr[0] = 10;
        {arr[1] = 20;}
        arr[2] = arr[0] + arr[1];
        return arr[2];
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.toInt() == 30;
}

int main() {
    auto run_test = [](std::string name, bool (*func)()) {
        if (func()) {
            std::cout << "✅ " << name << " success\n";
        } else {
            std::cerr << "❌ " << name << " failed\n";
        }
    };

    run_test("register_function_test", register_function_test);
    run_test("loop_math_test", loop_math_test);
    run_test("ternary_operator_test", ternary_operator_test);
    run_test("switch_case_test", switch_case_test);
    run_test("recursion_test", recursion_test);
    run_test("string_operation_test", string_operation_test);
    run_test("bitwise_operator_test", bitwise_operator_test);
    run_test("scope_shadowing_test", scope_shadowing_test);
    run_test("complex_math_priority_test", complex_math_priority_test);
    run_test("array_access_test", array_access_test);

    return 0;
}