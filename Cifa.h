#pragma once
#include <any>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace cifa
{
struct CalUnit;
class Cifa;

struct Object
{
    friend CalUnit;
    friend Cifa;

    Object() {}

    Object(double v)
    {
        value = v;
    }

    Object(double v, const std::string& t)
    {
        value = v;
        type1 = t;
    }

    Object(const std::string& str)
    {
        value = str;
    }

    Object(const std::string& str, const std::string& t)
    {
        value = str;
        type1 = t;
    }

    template <typename T, typename std::enable_if<!std::is_same_v<std::decay_t<T>, Object>, int>::type = 0>
    Object(const T& v)
    {
        value = v;
    }

    Object(int v)
    {
        value = double(v);
    }

    Object(bool v)
    {
        value = double(v);
    }

    operator bool() const { return toDouble() != 0; }

    operator int() const { return int(toDouble()); }

    operator double() const { return toDouble(); }

    operator std::string() const { return toString(); }

    bool toBool() const { return toDouble() != 0; }

    int toInt() const { return int(toDouble()); }

    double toDouble() const
    {
        if (value.type() == typeid(double))
        {
            return std::any_cast<double>(value);
        }
        fprintf(stderr, "Error: Object %s to double failed.\n", value.type().name());
        return NAN;
    }

    std::string toString() const
    {
        if (value.type() == typeid(std::string))
        {
            return std::any_cast<std::string>(value);
        }
        fprintf(stderr, "Error: Object %s to string failed.\n", value.type().name());
        return "";
    }

    //复制，不会改变原来的值
    template <typename T>
    T to() const
    {
        if (value.type() == typeid(T))
        {
            return std::any_cast<T>(value);
        }
        fprintf(stderr, "Error: Object %s to %s failed.\n", value.type().name(), typeid(T).name());
        return T();
    }

    //const与非const版本，按需使用
    //如果转换失败，后续使用时也不会正常，因此应谨慎使用，或者在isType()判断后使用
    template <typename T>
    const T& ref() const
    {
        if (value.type() == typeid(T))
        {
            return std::any_cast<const T&>(value);
        }
        fprintf(stderr, "Error: Object %s to %s failed.\n", value.type().name(), typeid(T).name());
    }

    template <typename T>
    T& ref()
    {
        if (value.type() == typeid(T))
        {
            return std::any_cast<T&>(value);
        }
        fprintf(stderr, "Error: Object %s to %s failed.\n", value.type().name(), typeid(T).name());
    }

    template <typename T>
    bool isType() const { return value.type() == typeid(T); }

    bool isNumber() const { return value.type() == typeid(double); }

    bool isEffectNumber() const { return isNumber() && !std::isnan(toDouble()) && !std::isinf(toDouble()); }

    bool hasValue() const { return value.has_value(); }

    const std::vector<Object>& subV() const { return v; }

    const std::string& getSpecialType() const { return type1; }

    std::type_info const& getType() const { return value.type(); }

private:
    std::any value;
    std::string type1;        //特别的类型，用于Error、break、continue
    std::vector<Object> v;    //仅用于处理逗号表达式
};

using ObjectVector = std::vector<Object>;

enum class CalUnitType
{
    None = 0,
    Constant,
    String,
    Operator,
    Split,
    Parameter,
    Function,
    Key,
    Type,
    Union,
    //UnionRound,    //()合并模式，仅for语句使用
};

struct CalUnit
{
    CalUnitType type = CalUnitType::None;
    std::vector<CalUnit> v;    //语法树的节点，v.size():[0,3]
    std::string str;
    size_t line = 0, col = 0;
    bool suffix = false;       //有后缀，可视为一个语句
    bool with_type = false;    //有前置的类型

    CalUnit(CalUnitType s, std::string s1)
    {
        type = s;
        str = s1;
    }

    CalUnit() {}

    bool can_cal()
    {
        return type == CalUnitType::Constant || type == CalUnitType::String || type == CalUnitType::Parameter || type == CalUnitType::Function || type == CalUnitType::Operator && v.size() > 0;
    }

    bool is_statement()
    {
        return suffix || !can_cal();
    }
};

struct Function2
{
    std::vector<std::string> arguments;
    CalUnit body;
};

template <typename T>
bool vector_have(const std::vector<T>& ops, const T& op)
{
    for (auto& o : ops)
    {
        if (op == o)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
bool vector_have(const std::vector<std::vector<T>>& ops, const T& op)
{
    for (auto& ops1 : ops)
    {
        for (auto& o : ops1)
        {
            if (op == o)
            {
                return true;
            }
        }
    }
    return false;
}

class Cifa
{
private:
    //运算符，此处的顺序即优先级，单目和右结合由下面的列表判断
    std::vector<std::vector<std::string>> ops = { { "::", ".", "++", "--" }, { "!" }, { "*", "/", "%" }, { "+", "-" }, { ">", "<", ">=", "<=" }, { "==", "!=" }, { "&" }, { "|" }, { "&&" }, { ":", "?" }, { "||" }, { "=", "*=", "/=", "+=", "-=" }, { "," } };
    std::vector<std::string> ops_single = { "++", "--", "!", "()++", "()--" };    //单目全部是右结合
    std::vector<std::string> ops_right = { "=", "*=", "/=", "+=", "-=" };         //右结合
    //关键字，在表中的位置为其所需参数个数
    std::vector<std::vector<std::string>> keys = { { "true", "false" }, { "break", "continue", "else", "return" }, { "if", "for", "while" } };
    std::vector<std::string> types = { "auto", "int", "float", "double" };

    //两个函数表都是全局的
    using func_type = std::function<Object(ObjectVector&)>;
    std::unordered_map<std::string, func_type> functions;     //在宿主程序中注册的函数
    std::unordered_map<std::string, Function2> functions2;    //在cifa程序中定义的函数

    std::unordered_map<std::string, void*> user_data;
    std::unordered_map<std::string, Object> parameters;    //变量表，注意每次定义的函数调用都是独立的

    struct ErrorMessage
    {
        size_t line = 0, col = 0;
        std::string message;
    };

    struct ErrorMessageComp
    {
        bool operator()(const ErrorMessage& l, const ErrorMessage& r) const
        {
            if (l.line == r.line)
            {
                return l.col < r.col;
            }
            return l.line < r.line;
        }
    };

    std::set<ErrorMessage, ErrorMessageComp> errors;

    bool output_error = true;

public:
    Cifa();
    Object eval(CalUnit& c, std::unordered_map<std::string, Object>& p);
    void expand_comma(CalUnit& c1, std::vector<CalUnit>& v);
    CalUnit& find_right_side(CalUnit& c1);
    //bool need_suffix(CalUnit& c) { return c.can_cal() || vector_have(keys[0], c.str); }

    CalUnitType guess_char(char c);
    std::list<CalUnit> split(std::string& str);

    CalUnit combine_all_cal(std::list<CalUnit>& ppp, bool curly = true, bool square = true, bool round = true);
    std::list<CalUnit>::iterator inside_bracket(std::list<CalUnit>& ppp, std::list<CalUnit>& ppp2, const std::string& bl, const std::string& br);
    void combine_curly_bracket(std::list<CalUnit>& ppp);
    void combine_square_bracket(std::list<CalUnit>& ppp);
    void combine_round_bracket(std::list<CalUnit>& ppp);
    void combine_ops(std::list<CalUnit>& ppp);
    void combine_semi(std::list<CalUnit>& ppp);
    void combine_keys(std::list<CalUnit>& ppp);
    void combine_types(std::list<CalUnit>& ppp);
    void combine_functions2(std::list<CalUnit>& ppp);

    void register_function(const std::string& name, func_type func);
    void register_user_data(const std::string& name, void* p);
    void register_parameter(const std::string& name, Object o);

    template <typename T>
    void register_parameter(const std::string& name, std::map<std::string, T> m)
    {
        //两重的暂时如此处理
        parameters[name] = "";
        for (auto& o : m)
        {
            parameters[name + "::" + o.first] = Object(o.second);
        }
    }

    template <typename T>
    void register_vector(const std::string& name, const std::vector<T>& v)
    {
        int i = 0;
        for (auto& o : v)
        {
            parameters[name + "[" + std::to_string(i++) + "]"] = Object(o);
        }
    }

    void* get_user_data(const std::string& name);
    Object run_function(const std::string& name, std::vector<CalUnit>& vc, std::unordered_map<std::string, Object>& p);
    Object& get_parameter(CalUnit& c, std::unordered_map<std::string, Object>& p);
    std::string convert_parameter_name(CalUnit& c, std::unordered_map<std::string, Object>& p);
    bool check_parameter(CalUnit& c, std::unordered_map<std::string, Object>& p);
    Object& get_parameter(const std::string& name, std::unordered_map<std::string, Object>& p);
    bool check_parameter(const std::string& name, std::unordered_map<std::string, Object>& p);

    void check_cal_unit(CalUnit& c, CalUnit* father, std::unordered_map<std::string, Object>& p);

    Object run_script(std::string str);    //运行脚本，注意实际上使用独立的变量表

    bool has_error() const { return !errors.empty(); }

    std::vector<ErrorMessage> get_errors() const;

    template <typename... Args>
    void add_error(CalUnit& c, Args... args)
    {
        ErrorMessage e;
        e.line = c.line;
        e.col = c.col;
        char buffer[1024] = { '\0' };
        if (sizeof...(args) == 1)
        {
            snprintf(buffer, 1024, "%s", args...);
        }
        else if (sizeof...(args) >= 2)
        {
            snprintf(buffer, 1024, args...);
        }
        e.message = buffer;
        errors.emplace(std::move(e));
    }

    void set_output_error(bool oe) { output_error = oe; }

    //四则运算准许用户增加自定义功能

#define OPERATOR(o1, o2, op, userop_v, trans_type) \
    if (o1.isNumber() && o2.isNumber()) \
    { \
        return double(trans_type(o1) op trans_type(o2)); \
    } \
    for (auto& f : userop_v) \
    { \
        auto o = f(o1, o2); \
        if (!o.isNumber()) \
        { \
            return o; \
        } \
    } \
    return Object();

#define OPERATOR_DEF(opname, op, trans_type) \
    Object opname(const Object& o1, const Object& o2) { OPERATOR(o1, o2, op, user_##opname, trans_type); }

#define OPERATOR_DEF_CONTENT(opname, op, trans_type) \
    Object opname(const Object& o1, const Object& o2) \
    { \
        if (o1.isType<std::string>() && o2.isType<std::string>()) \
        { \
            return Object(std::any_cast<std::string>(o1.value) + std::any_cast<std::string>(o2.value)); \
        } \
        OPERATOR(o1, o2, op, user_##opname, trans_type); \
    }

    std::vector<std::function<Object(const Object&, const Object&)>> user_add, user_sub, user_mul, user_div,
        user_less, user_more, user_less_equal, user_more_equal,
        user_equal, user_not_equal, user_bit_and, user_bit_or;

    OPERATOR_DEF_CONTENT(add, +, double)
    OPERATOR_DEF(sub, -, double)
    OPERATOR_DEF(mul, *, double)
    OPERATOR_DEF(div, /, double)
    OPERATOR_DEF(less, <, double)
    OPERATOR_DEF(more, >, double)
    OPERATOR_DEF_CONTENT(less_equal, <=, double)
    OPERATOR_DEF_CONTENT(more_equal, >=, double)
    OPERATOR_DEF_CONTENT(equal, ==, double)
    OPERATOR_DEF_CONTENT(not_equal, !=, double)
    OPERATOR_DEF(bit_and, &, int)
    OPERATOR_DEF(bit_or, |, int)
};

//#define OPERATOR_DEF_DOUBLE(op) \
//    Object op(const Object& o1, const Object& o2) { return Object(double(o1.value) op double(o2.value)); }

}    // namespace cifa