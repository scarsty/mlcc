#include <iostream>
#include <cmath>
#include <limits>
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
    }
    else {
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
        auto s1 = "Hello ";
        auto s2 = "World";
        return s1 + s2;
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.isType<std::string>() && o.toString() == "Hello World";
}

bool post_increment_test()
{   // 后置自增/自减测试
    Cifa c;
    std::string script = R"(
        int a = 10;
        int b = a++; 
        return b * 100 + a; // 期望 10 * 100 + 11 = 1011
    )";
    auto o = c.run_script(script);
    return o.hasValue() && o.toInt() == 1011;
}

int main() {
    if (register_function_test()) {
        std::cout << "✅register_function_test unit test success\n";
    } else {
        std::cerr << "❌register_function_test unit test failed\n";
    }

    if (loop_math_test()) {
        std::cout << "✅loop_math_test unit test success\n";
    } else {
        std::cerr << "❌loop_math_test unit test failed\n";
    }

    if (ternary_operator_test()) {
        std::cout << "✅ternary_operator_test unit test success\n";
    } else {
        std::cerr << "❌ternary_operator_test unit test failed\n";
    }

    if (switch_case_test()) {
        std::cout << "✅switch_case_test unit test success\n";
    } else {
        std::cerr << "❌switch_case_test unit test failed\n";
    }

    if (recursion_test()) {
        std::cout << "✅recursion_test unit test success\n";
    } else {
        std::cerr << "❌recursion_test unit test failed\n";
    }

    if (string_operation_test()) {
        std::cout << "✅string_operation_test unit test success\n";
    } else {
        std::cerr << "❌string_operation_test unit test failed\n";
    }

    if (post_increment_test()) {
        std::cout << "✅post_increment_test unit test success\n";
    } else {
        std::cerr << "❌post_increment_test unit test failed\n";
    }
}
