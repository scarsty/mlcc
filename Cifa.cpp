#include "Cifa.h"
#include <algorithm>
#include <iostream>
#include <sstream>

#define BREAK_POINT(content) \
    if (content) { int i = 0; }

namespace cifa
{

Cifa::Cifa()
{
    register_function("print", print);
    register_function("to_string", to_string);
    register_function("to_number", to_number);
    //parameters["true"] = Object(1, "__");
    //parameters["false"] = Object(0, "__");
    //parameters["break"] = Object("break", "__");
    //parameters["continue"] = Object("continue", "__");
    auto ifv = [](ObjectVector& x) -> Object
    {
        if (x.size() != 3) { return cifa::Object(); }
        int x0 = x[0];
        double x1 = x[1];
        double x2 = x[2];
        return (x0) ? x1 : x2;
    };
    register_function("ifv", ifv);
    register_function("ifvalue", ifv);

    register_function("abs", [](ObjectVector& x) -> Object
        {
            if (x.size() == 0) { return cifa::Object(); }
            double x0 = x[0];
            return abs(x0);
        });

    register_function("sqrt", [](ObjectVector& x) -> Object
        {
            if (x.size() == 0) { return cifa::Object(); }
            double x0 = x[0];
            return sqrt(x0);
        });

    register_function("pow", [](ObjectVector& x) -> Object
        {
            if (x.size() <= 1) { return cifa::Object(); }
            double x0 = x[0];
            double x1 = x[1];
            return pow(x0, x1);
        });

    register_function("round", [](ObjectVector& x) -> Object
        {
            if (x.size() == 0) { return cifa::Object(); }
            double x0 = x[0];
            return round(x0);
        });

    register_function("max", [](ObjectVector& x) -> Object
        {
            if (x.size() == 0) { return cifa::Object(); }
            if (x.size() == 1)
            {
                return x[0];
            }
            double max_val = x[0];
            for (int i = 1; i < x.size(); i++)
            {
                double v = x[i];
                if (max_val < v)
                {
                    max_val = v;
                }
            }
            return max_val;
        });

    register_function("min", [](ObjectVector& x) -> Object
        {
            if (x.size() == 0) { return cifa::Object(); }
            if (x.size() == 1)
            {
                return x[0];
            }
            double min_val = x[0];
            for (int i = 1; i < x.size(); i++)
            {
                double v = x[i];
                if (min_val > v)
                {
                    min_val = v;
                }
            }
            return min_val;
        });
}

Object Cifa::eval(CalUnit& c)
{
    if (force_return)
    {
        return Object();
    }
    else if (c.type == CalUnitType::Operator)
    {
        if (c.v.size() == 1)
        {
            if (c.str == "+") { return eval(c.v[0]); }
            if (c.str == "-") { return sub(Object(0), eval(c.v[0])); }
            if (c.str == "!") { return !eval(c.v[0]); }
            if (c.str == "++") { return get_parameter(c.v[0]) = add(get_parameter(c.v[0]), Object(1)); }
            if (c.str == "--") { return get_parameter(c.v[0]) = add(get_parameter(c.v[0]), Object(-1)); }
            if (c.str == "()++")
            {
                auto v = get_parameter(c);
                get_parameter(c.v[0]) = add(get_parameter(c.v[0]), Object(1));
                return v;
            }
            if (c.str == "()--")
            {
                auto v = get_parameter(c);
                get_parameter(c.v[0]) = add(get_parameter(c.v[0]), Object(-1));
                return v;
            }
        }
        if (c.v.size() == 2)
        {
            if (c.str == "." && c.v[0].can_cal())
            {
                if (c.v[1].type == CalUnitType::Function)
                {
                    if (c.v[1].v[0].type != CalUnitType::None)
                    {
                        std::vector<CalUnit> v = { c.v[0] };
                        expand_comma(c.v[1].v[0], v);
                        return run_function(c.v[1].str, v);
                    }
                    else
                    {
                        std::vector<CalUnit> v = { c.v[0] };
                        return run_function(c.v[1].str, v);
                    }
                }
            }
            if (c.str == "*") { return mul(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "/") { return div(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "%") { return int(eval(c.v[0])) % int(eval(c.v[1])); }
            if (c.str == "+") { return add(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "-") { return sub(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == ">") { return more(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "<") { return less(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == ">=") { return more_equal(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "<=") { return less_equal(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "==") { return equal(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "!=") { return not_equal(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "&") { return bit_and(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "|") { return bit_or(eval(c.v[0]), eval(c.v[1])); }
            if (c.str == "&&") { return bool(eval(c.v[0])) && bool(eval(c.v[1])); }
            if (c.str == "||") { return bool(eval(c.v[0])) || bool(eval(c.v[1])); }
            if (c.str == "=") { return get_parameter(c.v[0]) = eval(c.v[1]); }
            if (c.str == "+=") { return get_parameter(c.v[0]) = add(get_parameter(c.v[0]), eval(c.v[1])); }
            if (c.str == "-=") { return get_parameter(c.v[0]) = sub(get_parameter(c.v[0]), eval(c.v[1])); }
            if (c.str == "*=") { return get_parameter(c.v[0]) = mul(get_parameter(c.v[0]), eval(c.v[1])); }
            if (c.str == "/=") { return get_parameter(c.v[0]) = div(get_parameter(c.v[0]), eval(c.v[1])); }
            if (c.str == ",")
            {
                Object o;
                o.v.emplace_back(eval(c.v[0]));
                o.v.emplace_back(eval(c.v[1]));
                return o;
            }
        }
        return Object();
    }
    else if (c.type == CalUnitType::Constant)
    {
        return Object(atof(c.str.c_str()));
    }
    else if (c.type == CalUnitType::String)
    {
        return Object(c.str);
    }
    else if (c.type == CalUnitType::Parameter)
    {
        return get_parameter(c);
    }
    else if (c.type == CalUnitType::Function)
    {
        std::vector<CalUnit> v;
        if (!c.v.empty())
        {
            expand_comma(c.v[0], v);
        }
        return run_function(c.str, v);
    }
    else if (c.type == CalUnitType::Key)
    {
        if (c.str == "if")
        {
            if (eval(c.v[0]))
            {
                return eval(c.v[1]);
            }
            else if (c.v.size() >= 3)
            {
                return eval(c.v[2]);
            }
            return Object(0);
        }
        if (c.str == "for")
        {
            Object o;
            //此处v[0]应是一个组合语句
            for (eval(c.v[0].v[0]); eval(c.v[0].v[1]); eval(c.v[0].v[2]))
            {
                o = eval(c.v[1]);
                if (o.type == "__" && o.content == "break") { break; }
                if (o.type == "__" && o.content == "continue") { continue; }
                if (force_return) { return Object(); }
            }
            o.type = "";
            return o;
        }
        if (c.str == "while")
        {
            Object o;
            while (eval(c.v[0]))
            {
                o.v.emplace_back(eval(c.v[1]));
                if (o.type == "__" && o.content == "break") { break; }
                if (o.type == "__" && o.content == "continue") { continue; }
                if (force_return) { return Object(); }
            }
            o.type = "";
            return o;
        }
        if (c.str == "return")
        {
            result = eval(c.v[0]);
            force_return = true;
            return result;
        }
        if (c.str == "break")
        {
            return Object("break", "__");
        }
        if (c.str == "continue")
        {
            return Object("continue", "__");
        }
        if (c.str == "true")
        {
            return Object(1, "__");
        }
        if (c.str == "false")
        {
            return Object(0, "__");
        }
    }
    else if (c.type == CalUnitType::Union)
    {
        Object o;
        for (auto& c1 : c.v)
        {
            o = eval(c1);
            if (o.type == "__" && o.content == "break") { break; }
            if (o.type == "__" && o.content == "continue") { break; }
            if (force_return) { return Object(); }
        }
        return o;
    }
    return Object();
}

void Cifa::expand_comma(CalUnit& c1, std::vector<CalUnit>& v)
{
    if (c1.str == ",")
    {
        for (auto& c2 : c1.v)
        {
            expand_comma(c2, v);
        }
    }
    else
    {
        if (c1.type != CalUnitType::None)
        {
            v.push_back(c1);
        }
    }
};

CalUnit& Cifa::find_right_side(CalUnit& c1)
{
    CalUnit* p = &c1;
    while (p->v.size() > 0)
    {
        p = &(p->v.back());
    }
    return *p;
}

CalUnitType Cifa::guess_char(char c)
{
    if (std::string("0123456789").find(c) != std::string::npos)
    {
        return CalUnitType::Constant;
    }
    if (std::string("+-*/%=.!<>&|,").find(c) != std::string::npos)
    {
        return CalUnitType::Operator;
    }
    if (std::string("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:").find(c) != std::string::npos)
    {
        return CalUnitType::Parameter;
    }
    if (std::string("()[]{};").find(c) != std::string::npos)
    {
        return CalUnitType::Split;
    }
    if (std::string("\"\'").find(c) != std::string::npos)
    {
        return CalUnitType::String;
    }
    return CalUnitType::None;
}

//分割语法
std::list<CalUnit> Cifa::split(std::string& str)
{
    std::string r;
    std::list<CalUnit> rv;

    //删除注释
    size_t pos = 0;
    while (pos != std::string::npos)
    {
        if ((pos = str.find("/*", pos)) != std::string::npos)
        {
            auto pos1 = str.find("*/", pos + 2);
            if (pos1 == std::string::npos)
            {
                pos1 = str.size();
            }
            else
            {
                pos1 += 2;
            }
            for (size_t i = pos; i < pos1; i++)
            {
                if (str[i] != '\n') { str[i] = ' '; }
            }
        }
    }
    pos = 0;
    while (pos != std::string::npos)
    {
        if ((pos = str.find("//", pos)) != std::string::npos)
        {
            auto pos1 = str.find("\n", pos + 2);
            if (pos1 == std::string::npos)
            {
                pos1 = str.size();
            }
            std::fill(str.begin() + pos, str.begin() + pos1, ' ');
        }
    }

    CalUnitType stat = CalUnitType::None;
    char in_string = 0;
    char c0 = 0;
    size_t line = 1, col = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (i > 1) { c0 = str[i - 1]; }
        auto c = str[i];
        col++;
        auto pre_stat = stat;
        auto g = guess_char(c);
        if (in_string)
        {
            if (stat == CalUnitType::String && in_string == c)
            {
                in_string = 0;
                stat = CalUnitType::None;
            }
            else
            {
                stat = CalUnitType::String;
            }
        }
        else if (g == CalUnitType::String)
        {
            if (in_string == 0)
            {
                in_string = c;
                stat = CalUnitType::String;
            }
        }
        else if (g == CalUnitType::Constant)
        {
            if (stat == CalUnitType::Constant || stat == CalUnitType::Operator
                || stat == CalUnitType::Split || stat == CalUnitType::None)
            {
                stat = CalUnitType::Constant;
            }
            else if (stat == CalUnitType::Parameter)
            {
                stat = CalUnitType::Parameter;
            }
        }
        else if (g == CalUnitType::Operator)
        {
            if (c == '.' && stat == CalUnitType::Constant)
            {
            }
            else if ((c == '+' || c == '-') && (c0 == 'E' || c0 == 'e') && stat == CalUnitType::Constant)
            {
            }
            else
            {
                stat = CalUnitType::Operator;
            }
        }
        else if (g == CalUnitType::Split)
        {
            stat = CalUnitType::Split;
        }
        else if (g == CalUnitType::Parameter)
        {
            if ((c == 'E' || c == 'e') && stat == CalUnitType::Constant)
            {
            }
            else
            {
                stat = CalUnitType::Parameter;
            }
        }
        else if (g == CalUnitType::None)
        {
            stat = CalUnitType::None;
        }
        if (pre_stat != stat || stat == CalUnitType::Operator || stat == CalUnitType::Split)
        {
            if (pre_stat != CalUnitType::None)
            {
                CalUnit c(pre_stat, r);
                c.line = line;
                c.col = col - r.size();
                rv.emplace_back(std::move(c));
            }
            r.clear();
            if (g != CalUnitType::String)
            {
                r = c;
            }
        }
        else
        {
            r += c;
        }
        if (c == '\n')
        {
            col = 0;
            line++;
        }
    }
    if (stat != CalUnitType::None)
    {
        CalUnit c(stat, r);
        c.line = line;
        c.col = col - r.size();
        rv.emplace_back(std::move(c));
    }

    for (auto it = rv.begin(); it != rv.end(); ++it)
    {
        //括号前的变量视为函数
        if (it->str == "(")
        {
            if (it != rv.begin() && std::prev(it)->type == CalUnitType::Parameter && functions.count(std::prev(it)->str) > 0)
            {
                std::prev(it)->type = CalUnitType::Function;
            }
        }
        for (auto& keys1 : keys)
        {
            if (vector_have(keys1, it->str))
            {
                it->type = CalUnitType::Key;
            }
        }
        if (vector_have(types, it->str))
        {
            it->type = CalUnitType::Type;
        }
    }

    //合并多字节运算符
    for (auto& ops1 : ops)
    {
        for (auto& op : ops1)
        {
            if (op.size() == 2)
            {
                for (auto it = rv.begin(); it != rv.end();)
                {
                    auto itr = std::next(it);
                    if (itr != rv.end()
                        && it->str == std::string(1, op[0]) && itr->str == std::string(1, op[1])
                        && it->line == itr->line && it->col == itr->col - 1)
                    {
                        it->str = op;
                        it = rv.erase(std::next(it));
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }

    //不处理类型符号
    for (auto it = rv.begin(); it != rv.end();)
    {
        if (it->type == CalUnitType::Type)
        {
            it = rv.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return rv;
}

//表达式语法树
//参数含义：是否合并{}，是否合并[]，是否合并()
CalUnit Cifa::combine_all_cal(std::list<CalUnit>& ppp, bool curly, bool square, bool round)
{
    //合并{}
    if (curly) { combine_curly_backet(ppp); }
    //合并[]
    if (square) { combine_square_backet(ppp); }
    //合并()
    if (round) { combine_round_backet(ppp); }

    //合并算符
    combine_ops(ppp);

    //检查分号正确性并去除
    combine_semi(ppp);

    //合并关键字
    combine_keys(ppp);

    //到此处应仅剩余单独分号（空语句）、语句、语句组，只需简单合并即可
    //即使只有一条语句也必须返回Union！
    CalUnit c;
    c.type = CalUnitType::Union;
    if (ppp.size() == 0)
    {
        return c;
    }
    else
    {
        c.line = ppp.front().line;
        c.col = ppp.front().col;
        for (auto it = ppp.begin(); it != ppp.end(); ++it)
        {
            if (it->type != CalUnitType::Split)
            {
                c.v.emplace_back(std::move(*it));
            }
        }
        return c;
    }
}

//查找现有最内层括号，并返回位置
std::list<CalUnit>::iterator Cifa::inside_bracket(std::list<CalUnit>& ppp, std::list<CalUnit>& ppp2, const std::string& bl, const std::string& br)
{
    bool have = false;
    auto it = ppp.begin();
    for (; it != ppp.end(); ++it)
    {
        if (it->str == bl || it->str == br)
        {
            have = true;
            break;
        }
    }
    if (!have)
    {
        return ppp.end();
    }
    if (it->str == br)
    {
        add_error(*it, "unpaired right bracket %s", it->str.c_str());
        return ppp.end();
    }
    auto itl0 = it, itr0 = ppp.end();
    for (auto itr = it; itr != ppp.end(); ++itr)
    {
        if (itr->str == br)
        {
            itr0 = itr;
            for (auto itl = std::prev(itr); itl != ppp.begin(); --itl)
            {
                if (itl->str == bl)
                {
                    itl0 = itl;
                    break;
                }
            }
            break;
        }
    }
    if (itr0 == ppp.end())
    {
        add_error(*it, "unpaired left bracket %s", it->str.c_str());
        return ppp.end();
    }
    ppp2.splice(ppp2.begin(), ppp, std::next(itl0), itr0);
    return itl0;
}

void Cifa::combine_curly_backet(std::list<CalUnit>& ppp)
{
    while (true)
    {
        std::list<CalUnit> ppp2;
        auto it = inside_bracket(ppp, ppp2, "{", "}");
        if (it == ppp.end())
        {
            break;
        }
        auto c1 = combine_all_cal(ppp2, false, true, true);    //此处合并多行
        c1.str = "{}";
        c1.line = it->line;
        c1.col = it->col;
        it = ppp.erase(it);
        *it = std::move(c1);
    }
}

void Cifa::combine_square_backet(std::list<CalUnit>& ppp)
{
    while (true)
    {
        std::list<CalUnit> ppp2;
        auto it = inside_bracket(ppp, ppp2, "[", "]");
        if (it == ppp.end())
        {
            break;
        }
        auto c1 = combine_all_cal(ppp2, true, false, true);
        c1.str = "[]";
        c1.line = it->line;
        c1.col = it->col;
        it = ppp.erase(it);
        *it = std::move(c1);
        if (it != ppp.begin())
        {
            if (std::prev(it)->type == CalUnitType::Parameter)
            {
                std::prev(it)->v = { *it };
                ppp.erase(it);
            }
        }
    }
}

void Cifa::combine_round_backet(std::list<CalUnit>& ppp)
{
    while (true)
    {
        std::list<CalUnit> ppp2;
        //auto size = ppp.size();
        auto it = inside_bracket(ppp, ppp2, "(", ")");
        if (it == ppp.end())
        {
            break;
        }
        it = ppp.erase(it);
        auto c1 = combine_all_cal(ppp2, true, true, false);
        c1.str = "()";
        c1.line = it->line;
        c1.col = it->col;
        if (c1.v.size() == 0)
        {
            it->type = CalUnitType::None;
        }
        else if (c1.v.size() == 1)
        {
            *it = std::move(c1.v[0]);
        }
        else
        {
            *it = std::move(c1);
        }
        //括号前
        if (it != ppp.begin())
        {
            auto itl = std::prev(it);
            if (itl->type == CalUnitType::Function || itl->type == CalUnitType::Parameter || itl->type == CalUnitType::Constant)
            {
                itl->v = { std::move(*it) };
                ppp.erase(it);
            }
            else if (itl->type == CalUnitType::Key && vector_have(keys[2], itl->str))
            {
                itl->v = { std::move(*it) };
                ppp.erase(it);
            }
        }
    }
}

void Cifa::combine_ops(std::list<CalUnit>& ppp)
{
    for (const auto& ops1 : ops)
    {
        for (auto& op : ops1)
        {
            bool is_right = false;
            if (vector_have(ops_single, op) || vector_have(ops_right, op) || op == "+" || op == "-")    //右结合
            {
                auto it = ppp.end();
                for (; it != ppp.begin();)
                {
                    --it;
                    if (it->type == CalUnitType::Operator && it->str == op && it->v.size() == 0)
                    {
                        BREAK_POINT(it->str == "++" || it->str == "--");
                        if (it == ppp.begin() || vector_have(ops_single, it->str)
                            || !std::prev(it)->can_cal() && (op == "+" || op == "-"))    //+-退化为单目运算的情况
                        {
                            is_right = true;
                            auto itr = std::next(it);
                            bool is_single = false;
                            if (itr != ppp.end())
                            {
                                if ((it->str == "+" || it->str == "-")
                                        && (itr->type == CalUnitType::Constant || itr->type == CalUnitType::Function || itr->type == CalUnitType::Parameter)
                                    || (it->str == "++" || it->str == "--")
                                        && itr->type == CalUnitType::Parameter)
                                {
                                    it->v = { std::move(*itr) };
                                    it = ppp.erase(itr);
                                    is_single = true;
                                }                                
                            }
                            if (!is_single && it != ppp.begin() && (it->str == "++" || it->str == "--") && std::prev(it)->type == CalUnitType::Parameter)
                            {
                                it->v = { std::move(*std::prev(it)) };
                                it->str = "()" + it->str;
                                it = ppp.erase(std::prev(it));
                            }
                        }
                        else
                        {
                            if (it->str != "+" && it->str != "-")
                            {
                                is_right = true;
                                auto itr = std::next(it);
                                if (itr != ppp.end())
                                {
                                    it->v = { std::move(*std::prev(it)), std::move(*itr) };
                                    ppp.erase(itr);
                                    it = ppp.erase(std::prev(it));
                                }
                            }
                        }
                    }
                }
            }
            if (!is_right)    //未能成功右结合则判断为左结合
            {
                for (auto it = ppp.begin(); it != ppp.end();)
                {
                    if (it->type == CalUnitType::Operator && it->str == op && it->v.size() == 0 && it != ppp.begin())
                    {
                        auto itr = std::next(it);
                        if (itr != ppp.end())
                        {
                            it->v = { std::move(*std::prev(it)), std::move(*itr) };
                            ppp.erase(itr);
                            it = ppp.erase(std::prev(it));
                        }
                    }
                    ++it;
                }
            }
        }
    }
}

void Cifa::combine_semi(std::list<CalUnit>& ppp)
{
    for (auto it = ppp.begin(); it != ppp.end();)
    {
        if (it->can_cal())
        {
            auto itr = std::next(it);
            if (itr != ppp.end() && itr->str == ";")
            {
                it->suffix = true;
                it = ppp.erase(itr);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

void Cifa::combine_keys(std::list<CalUnit>& ppp)
{
    //合并关键字，从右向左
    auto it = ppp.end();
    while (it != ppp.begin())
    {
        --it;
        for (size_t para_count = 1; para_count < keys.size(); para_count++)
        {
            auto& keys1 = keys[para_count];
            if (it->type == CalUnitType::Key && it->v.size() < para_count && vector_have(keys1, it->str))
            {
                auto itr = std::next(it);
                if (itr != ppp.end())
                {
                    it->v.emplace_back(std::move(*itr));
                    itr = ppp.erase(itr);
                }
            }
            if (it->str == "if" && it->v.size() == 2 && std::next(it) != ppp.end())
            {
                auto itr = std::next(it);
                if (itr->str == "else")
                {
                    it->v.emplace_back(std::move(*itr));
                    ppp.erase(itr);
                    if (it->v[2].v.size() > 0)
                    {
                        auto c = std::move(it->v[2].v[0]);
                        it->v[2] = std::move(c);
                    }
                }
            }
        }
    }
}

void Cifa::combine_types(std::list<CalUnit>& ppp) {}

void Cifa::register_function(const std::string& name, func_type func)
{
    functions[name] = func;
}

void Cifa::register_user_data(const std::string& name, void* p)
{
    user_data[name] = p;
}

void Cifa::register_parameter(const std::string& name, Object o)
{
    parameters[name] = o;
}

void* Cifa::get_user_data(const std::string& name)
{
    return user_data[name];
}

Object Cifa::run_function(const std::string& name, std::vector<CalUnit>& vc)
{
    if (functions.count(name))
    {
        auto p = functions[name];
        std::vector<Object> v;
        for (auto& c : vc)
        {
            v.emplace_back(eval(c));
        }
        return p(v);
    }
    else
    {
        return Object();
    }
}

Object& Cifa::get_parameter(CalUnit& c)
{
    return parameters[convert_parameter_name(c)];
}

std::string Cifa::convert_parameter_name(CalUnit& c)
{
    std::string parameter_name = c.str;
    if (c.v.size() > 0 && c.v[0].str == "[]")
    {
        if (c.v[0].v.size() > 0)
        {
            auto e = eval(c.v[0].v[0]);
            std::string str;
            if (e.type == "string")
            {
                str = e.content;
            }
            else
            {
                str = std::to_string(int(e.value));
            }
            parameter_name += "[" + str + "]";
        }
    }
    return parameter_name;
}

bool Cifa::check_parameter(CalUnit& c)
{
    return parameters.count(convert_parameter_name(c));
}

void Cifa::check_cal_unit(CalUnit& c, CalUnit* father)
{
    //若提前return，表示不再检查其下的结构
    if (c.type == CalUnitType::Operator)
    {
        if (vector_have(ops_single, c.str))
        {
            if (c.v.size() != 1)
            {
                add_error(c, "operator %s has wrong operands", c.str.c_str());
            }
        }
        else if (vector_have(ops, c.str) && !vector_have(ops_single, c.str))
        {
            if (c.str == "=")
            {
                if (c.v[0].type == CalUnitType::Parameter)
                {
                    check_cal_unit(c.v[1], &c);    //here make sure no undefined parameters at right of "="
                    get_parameter(c.v[0]);         //record a parameter at left of "="
                    //this will check twice things at right of "="
                }
                if (c.v[0].type == CalUnitType::Parameter && get_parameter(c.v[0]).type == "__"
                    || c.v[0].type != CalUnitType::Parameter)
                {
                    add_error(c, "%s cannot be assigned", c.v[0].str.c_str());
                }
            }
            if (c.v.size() == 1 && (c.str == "+" || c.str == "-"))
            {
            }
            else if (c.v.size() != 2)
            {
                add_error(c, "operator %s has wrong operands", c.str.c_str());
            }
        }
        else
        {
            add_error(c, "unknown operator %s with %zu operands", c.str.c_str(), c.v.size());
        }
    }
    else if (c.type == CalUnitType::Constant || c.type == CalUnitType::String)
    {
        if (c.v.size() > 0)
        {
            add_error(c, "cannot calculate constant %s with operands", c.str.c_str());
        };
    }
    else if (c.type == CalUnitType::Parameter)
    {
        if (c.v.size() > 0 && c.v[0].str != "[]")
        {
            add_error(c, "cannot calculate parameter %s with operands", c.str.c_str());
        }
        if (!check_parameter(c))
        {
            add_error(c, "parameter %s is at right of = but not been initialized", c.str.c_str());    //parameters at left of "=" have been added
        }
    }
    else if (c.type == CalUnitType::Function)
    {
        if (c.v.size() == 0)
        {
            add_error(c, "function %s has no operands", c.str.c_str());
        }
    }
    else if (c.type == CalUnitType::Key)
    {
        if (c.str == "if")
        {
            if (c.v.size() == 0)
            {
                add_error(c, "if has no condition");
            }
            if (c.v.size() == 1)
            {
                add_error(c, "if has no statement");
            }
            if (c.v.size() >= 2 && !c.v[1].is_statement())
            {
                add_error(c.v[1], "missing ;");
            }
            if (c.v.size() >= 3)
            {
                if (c.v[2].str == "else")
                {
                    add_error(c.v[2], "else has no if");
                }
                else if (!c.v[2].is_statement())
                {
                    add_error(c.v[2], "missing ;");
                }
            }
        }
        if (c.str == "else")    //语法树合并后不应有单独的else
        {
            add_error(c, "else has no if");
        }
        if (c.str == "for")
        {
            if (c.v[0].type != CalUnitType::Union || c.v[0].str != "()" || c.v[0].v.size() != 3
                || !c.v[0].v[0].is_statement() || !c.v[0].v[1].is_statement() || c.v[0].v[2].is_statement())
            {
                add_error(c, "for loop condition is not right");
            }
            if (c.v.size() >= 2 && !c.v[1].is_statement())
            {
                add_error(c.v[1], "missing ;");
            }
        }
        if (c.str == "while")
        {
            if (c.v.size() == 0)
            {
                add_error(c, "while has no condition");
            }
            if (c.v.size() == 1)
            {
                add_error(c, "while has no statement");
            }
            if (c.v.size() >= 2 && !c.v[1].is_statement())
            {
                add_error(c.v[1], "missing ;");
            }
        }
        if (c.str == "return")
        {
            if (c.v.size() == 0 || !c.v[0].is_statement())
            {
                add_error(c, "%s missing ;", c.str.c_str());
            }
        }
        if (c.str == "break" || c.str == "continue")
        {
            if (c.v.size() == 0 || c.v[0].str != ";")
            {
                add_error(c, "%s missing ;", c.str.c_str());
            }
        }
    }
    else if (c.type == CalUnitType::Union)
    {
        if (father == nullptr || c.str == "{}" && (father->type == CalUnitType::Union || father->type == CalUnitType::Key))
        {
            for (auto& c1 : c.v)
            {
                if (!c1.is_statement())
                {
                    add_error(c1, "missing ;");
                }
            }
        }
        if (c.str == "[]")
        {
            if (c.v.size() == 0 || c.v[0].str == ",")
            {
                add_error(c, "wrong parameters inside []");
            }
        }
        if (c.str == "()")
        {
            if ((father == nullptr || (father != nullptr && father->str != "for")) && c.v.size() > 1)
            {
                add_error(c, "wrong parameters inside ()");
            }
        }
    }
    for (auto& c1 : c.v)
    {
        check_cal_unit(c1, &c);
    }
}

Object Cifa::run_script(std::string str)
{
    errors.clear();
    force_return = false;
    result = Object();

    str += ";";    //方便处理仅有一行的情况
    auto rv = split(str);
    auto c = combine_all_cal(rv);    //结果必定是一个Union
    if (errors.empty())
    {
        check_cal_unit(c, nullptr);
    }
    if (errors.empty())
    {
        auto o = eval(c);
        //如果只有一个表达式，则返回其值，否则为return返回的值
        if (c.v.size() <= 1)
        {
            return o;
        }
    }
    else
    {
        result.type = "Error";
        for (auto& e : errors)
        {
            result.content += "Error (" + std::to_string(e.line) + ", " + std::to_string(e.col) + "): " + e.message + "\n";
        }
        if (output_error)
        {
            std::cerr << result.content;
        }
    }
    return result;
}

Object print(ObjectVector& d)
{
    for (auto& d1 : d)
    {
        if (d1.type == "string")
        {
            std::cout << d1.content;
        }
        else
        {
            std::cout << d1.value;
        }
    }
    std::cout << "\n";
    return Object(double(d.size()));
}

Object to_string(ObjectVector& d)
{
    if (d.empty())
    {
        return Object("");
    }
    std::ostringstream stream;
    stream << d[0].value;
    return Object(stream.str());
}

Object to_number(ObjectVector& d)
{
    if (d.empty())
    {
        return Object();
    }
    return Object(atof(d[0].content.c_str()));
}

}    // namespace cifa