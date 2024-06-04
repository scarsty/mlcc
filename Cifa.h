#pragma once
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace cifa
{

struct Object
{
    Object() {}
    Object(double v, const std::string& ty = "")
    {
        value = v;
        type = ty;
    }
    Object(const std::string& str, const std::string& ty = "string")
    {
        content = str;
        type = ty;
    }
    double value = nan("");
    std::string content;
    std::string type;
    std::vector<Object> v;
    operator bool() { return value; }
    operator int() { return value; }
    operator double() { return value; }
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
    std::vector<CalUnit> v;
    std::string str;
    size_t line = 0, col = 0;
    bool suffix = false;
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
    std::vector<std::vector<std::string>> ops = { { ".", "++", "--" }, { "!" }, { "*", "/", "%" }, { "+", "-" }, { ">", "<", ">=", "<=" }, { "==", "!=" }, { "&" }, { "|" }, { "&&" }, { "||" }, { "=", "*=", "/=", "+=", "-=" }, { "," } };
    std::vector<std::string> ops_single = { "++", "--", "!", "()++", "()--" };    //单目全部是右结合
    std::vector<std::string> ops_right = { "=", "*=", "/=", "+=", "-=" };         //右结合
    //关键字，在表中的位置为其所需参数个数
    std::vector<std::vector<std::string>> keys = { { "true", "false" }, { "break", "continue", "else", "return" }, { "if", "for", "while" } };
    std::vector<std::string> types = { "auto", "int", "float", "double" };

    std::map<std::string, void*> user_data;
    std::map<std::string, Object> parameters;
    using func_type = std::function<Object(ObjectVector&)>;
    std::map<std::string, func_type> functions;

    bool force_return = false;
    Object result;

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
    Object eval(CalUnit& c);
    void expand_comma(CalUnit& c1, std::vector<CalUnit>& v);
    CalUnit& find_right_side(CalUnit& c1);
    //bool need_suffix(CalUnit& c) { return c.can_cal() || vector_have(keys[0], c.str); }

    CalUnitType guess_char(char c);
    std::list<CalUnit> split(std::string& str);

    CalUnit combine_all_cal(std::list<CalUnit>& ppp, bool curly = true, bool square = true, bool round = true);
    std::list<CalUnit>::iterator inside_bracket(std::list<CalUnit>& ppp, std::list<CalUnit>& ppp2, const std::string& bl, const std::string& br);
    void combine_curly_backet(std::list<CalUnit>& ppp);
    void combine_square_backet(std::list<CalUnit>& ppp);
    void combine_round_backet(std::list<CalUnit>& ppp);
    void combine_ops(std::list<CalUnit>& ppp);
    void combine_semi(std::list<CalUnit>& ppp);
    void combine_keys(std::list<CalUnit>& ppp);
    void combine_types(std::list<CalUnit>& ppp);

    void register_function(const std::string& name, func_type func);
    void register_user_data(const std::string& name, void* p);
    void register_parameter(const std::string& name, Object o);
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
    Object run_function(const std::string& name, std::vector<CalUnit>& vc);
    Object& get_parameter(CalUnit& c);
    std::string convert_parameter_name(CalUnit& c);
    bool check_parameter(CalUnit& c);

    void check_cal_unit(CalUnit& c, CalUnit* father);

    Object run_script(std::string str);

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

    void setOutputError(bool oe) { output_error = oe; }

    //四则运算准许用户增加自定义功能

#define OPERATOR(o1, o2, op, op2, trans_type) \
    if (o1.type == "" && o2.type == "") \
    { \
        return Object(trans_type(o1.value) op trans_type(o2.value)); \
    } \
    if (op2) \
    { \
        return op2(o1, o2); \
    } \
    return Object(trans_type(o1.value) op trans_type(o2.value));

#define OPERATOR_DEF(name, op, trans_type) \
    Object name(const Object& o1, const Object& o2) { OPERATOR(o1, o2, op, user_##name, trans_type); }

#define OPERATOR_DEF_CONTENT(name, op, trans_type) \
    Object name(const Object& o1, const Object& o2) \
    { \
        if (o1.type == "string" && o2.type == "string") \
        { \
            return Object(o1.content + o2.content); \
        } \
        OPERATOR(o1, o2, op, user_##name, trans_type); \
    }

    std::function<Object(const Object&, const Object&)> user_add, user_sub, user_mul, user_div,
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

Object print(ObjectVector& d);
Object to_string(ObjectVector& d);
Object to_number(ObjectVector& d);

}    // namespace cifa