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

bool loop_math_test() {
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

int main() {
    if (register_function_test()) {
        std::cout << "✅register_function_test unit test success\n";
    } else {
        std::cerr << "❌register_function_test unit test failed\n";
    }

    if (loop_math_test()) {
        std::cout << "✅loop_math_test unit test success\n";
    }
    else {
        std::cerr << "❌loop_math_test unit test failed\n";
    }
}
