#pragma once
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

class FakeJson
{
private:
    std::variant<nullptr_t, int64_t, double, bool, std::string, std::map<std::string, FakeJson>, std::vector<FakeJson>> value;
    std::map<std::string, FakeJson>& value_map()
    {
        if (!isMap())
        {
            value = std::map<std::string, FakeJson>();
        }
        return std::get<std::map<std::string, FakeJson>>(value);
    }
    std::vector<FakeJson>& value_vector()
    {
        if (!isVector())
        {
            value = std::vector<FakeJson>();
        }
        return std::get<std::vector<FakeJson>>(value);
    }
    enum
    {
        type_null,
        type_int,
        type_double,
        type_bool,
        type_string,
        type_map,
        type_vector
    };

public:
    FakeJson() = default;

    FakeJson(int v) { value = v; }

    FakeJson(double v) { value = v; }

    FakeJson(const std::string& v) { value = v; }

    FakeJson(const char* v) { value = std::string(v); }

    FakeJson(bool v) { value = v; }

    template <typename T>
    FakeJson(const std::vector<T>& v)
    {
        for (auto& v1 : v)
        {
            pushBack(v1);
        }
    }

    template <typename T>
    FakeJson(const std::map<std::string, T>& m)
    {
        for (auto& p : m)
        {
            (*this)[p.first] = p.second;
        }
    }

    template <typename T>
    FakeJson(const std::pair<std::string, T>& p) { (*this)[p.first] = p.second; }

    template <typename T>
    FakeJson(const std::pair<const char*, T>& p) { (*this)[p.first] = p.second; }

    //std::type_info const& type() const { return value.type(); }

    template <typename T>
    bool isType() const { return std::holds_alternative<T>(value); }

    bool isInt() const { return value.index() == type_int; }

    bool isDouble() const { return value.index() == type_double; }

    bool isString() const { return value.index() == type_string; }

    bool isBool() const { return value.index() == type_bool; }

    bool isNull() const { return value.index() == type_null; }

    bool isValue() const { return value.index() != type_null && value.index() != type_map && value.index() != type_vector; }

    bool isMap() const { return value.index() == type_map; }

    bool isVector() const { return value.index() == type_vector; }

    int toInt() { return std::get<int64_t>(value); }

    double toDouble() { return std::get<double>(value); }

    std::string toString() { return std::get<std::string>(value); }

    bool toBool() { return std::get<bool>(value); }

    FakeJson& operator[](int v) { return value_vector()[v]; }

    FakeJson& operator[](const std::string& v) { return value_map()[v]; }

    FakeJson& operator[](const char* v) { return value_map()[std::string(v)]; }

    void pushBack(const FakeJson& v) { value_vector().push_back(v); }

    void erase(const std::string& v) { value_map().erase(v); }    //需由上一级删除，而不能自己删除，下同

    void erase(int v)
    {
        if (v >= 0 && v < value_vector().size()) { value_vector().erase(value_vector().begin() + v); }
    }

    void clear()
    {
        value = nullptr;
    }

    bool exist(const std::string& v) const { return std::get<std::map<std::string, FakeJson>>(value).count(v); }

    bool exist(int v) const { return v >= 0 && v < std::get<std::vector<FakeJson>>(value).size(); }

    //template <typename T>
    //std::vector<T> toVector() const
    //{
    //    std::vector<T> v;
    //    for (auto& i : value_vector)
    //    {
    //        v.emplace_back(i.to<T>());
    //    }
    //    return v;
    //}

    //template <typename T>
    //std::map<std::string, T> toMap() const
    //{
    //    std::map<std::string, T> v;
    //    for (auto& i : value_map)
    //    {
    //        v[i.first] = i.second.to<T>();
    //    }
    //    return v;
    //}

    bool isPrintable() const
    {
        return isInt() || isDouble() || isString() || isBool() || isNull();
    }

    bool isNum() const { return isInt() || isDouble(); }

    std::string to_string(bool narrow = true, int space = 0) const
    {
        if (isInt())
        {
            return std::to_string(std::get<int64_t>(value));
        }
        if (isDouble())
        {
            return std::to_string(std::get<double>(value));
        }
        if (isString())
        {
            return "\"" + std::get<std::string>(value) + "\"";
        }
        if (isBool())
        {
            return std::get<bool>(value) ? "true" : "false";
        }
        if (isNull())
        {
            return "null";
        }
        if (isMap())
        {
            std::string res;
            for (auto& [k, v] : std::get<std::map<std::string, FakeJson>>(value))
            {
                if (!narrow)
                    {
                    res += std::string(space+4, ' ');
                    }
                res += "\"" + k + "\": " + v.to_string(narrow, space+4) + ", ";
                if (!narrow)
                {
                    res += "\n";
                }
            }
            if (!res.empty())
            {
                res.pop_back();
                res.pop_back();
                if (!narrow)
                {
                    res.pop_back();
                }
            }
            if (narrow)
            {
                res = "{" + res + "}";
            }
            else
            {
                res = "\n"+ std::string(space, ' ') + "{\n" + res + "\n" + std::string(space , ' ') + "}";
            }
            return res;
        }
        if (isVector())
        {
            std::string res;
            for (auto& i : std::get<std::vector<FakeJson>>(value))
            {
                if (!narrow)
                {
                    res += std::string(space + 4, ' ');
                }
                res += i.to_string(narrow, space+4) + ", ";
                if (!narrow)
                {
                    res += "\n";
                }
            }
            if (!res.empty())
            {
                res.pop_back();
                res.pop_back();
                if (!narrow)
                {
                    res.pop_back();
                }
            }
            if (narrow)
            {
                res = "[" + res + "]";
            }
            else
            {
                res = "\n" + std::string(space, ' ') + "[\n" + res + "\n" + std::string(space, ' ') + "]";
            }
            return res;
        }

        return "";
    }

    void parse(const std::string& str)
    {
        FakeJson& o = *this;
        std::vector<FakeJson*> ptr{ &o };
        int ignore_space = 1;
        char quote = '\0';
        bool found_backslash = false;
        std::string cur;

        auto try_to_variant = [](std::string&& str1) -> decltype(value)
        {
            auto str = std::move(str1);
            str1.clear();
            if (str.empty())
            {
                return nullptr;
            }
            if (str[0] == '\"' && str.back() == '\"')
            {
                return str.substr(1, str.size() - 2);
            }
            if (str[0] == '\'' && str.back() == '\'')
            {
                return str.substr(1, str.size() - 2);
            }
            if (str == "true")
            {
                return true;
            }
            if (str == "false")
            {
                return false;
            }
            if (str == "null")
            {
                return nullptr;
            }
            char* end = nullptr;
            auto i = strtoll(str.c_str(), &end, 10);
            if (end == str.c_str() + str.size())
            {
                return int(i);
            }
            char* end2 = nullptr;
            auto d = strtod(str.c_str(), &end2);
            if (end2 == str.c_str() + str.size())
            {
                return d;
            }
            return std::move(str);
        };

        auto dequote = [](std::string& str) -> std::string
        {
            if (str.empty())
            {
                return str;
            }
            if (str[0] == '\"' && str.back() == '\"')
            {
                return str.substr(1, str.size() - 2);
            }
            if (str[0] == '\'' && str.back() == '\'')
            {
                return str.substr(1, str.size() - 2);
            }
            return str;
        };

        for (auto& c : str)
        {
            if (c == '\"' || c == '\'')
            {
                if (!found_backslash)
                {
                    if (quote == '\0')
                    {
                        quote = c;
                        //continue;
                    }
                    else if (quote == c)
                    {
                        quote = '\0';
                        //continue;
                    }
                }
            }
            if (quote == '\0')
            {
                if (c == '[')
                {
                    ptr.push_back(&ptr.back()->value_vector().emplace_back());
                    ignore_space = 1;
                }
                else if (c == ']')
                {
                    if (ptr.back()->isNull())
                    {
                        ptr.back()->value = try_to_variant(std::move(cur));
                    }
                    if (ptr.size() >= 2)
                    {
                        ptr.pop_back();
                    }
                }
                else if (c == '{')
                {
                    ptr.push_back(&ptr.back()->value_map()[""]);
                    ignore_space = 1;
                }
                else if (c == '}')
                {
                    if (ptr.back()->isNull())
                    {
                        ptr.back()->value = try_to_variant(std::move(cur));
                    }
                    ptr[ptr.size() - 2]->value_map().erase("");
                    if (ptr.size() >= 2)
                    {
                        ptr.pop_back();
                    }
                }
                else if (c == ':')
                {
                    ptr.back() = &ptr[ptr.size() - 2]->value_map()[dequote(cur)];
                    cur.clear();
                    ignore_space = 1;
                }
                else if (c == ',')
                {
                    if (ptr.back()->isValue() || ptr.back()->isNull())
                    {
                        ptr.back()->value = try_to_variant(std::move(cur));
                    }
                    if (ptr.size() >= 2)
                    {
                        if (ptr[ptr.size() - 2]->isVector())
                        {
                            ptr.back() = &ptr[ptr.size() - 2]->value_vector().emplace_back();
                        }
                        if (ptr[ptr.size() - 2]->isMap())
                        {
                            ptr.back() = &ptr[ptr.size() - 2]->value_map()[""];
                        }
                    }
                    ignore_space = 1;
                }
                else if ((c == ' ' || c == '\n' || c == '\r') && ignore_space == 1)
                {
                }
                else
                {
                    cur += c;
                    ignore_space = 1;
                    //如果是非紧凑格式，需要忽略这里的空格
                    //因此转为非json字串时，string必须用引号包围
                }
            }
            else
            {
                if (c == '\\' && !found_backslash)
                {
                    found_backslash = true;
                    continue;
                }
                else if (found_backslash)
                {
                    found_backslash = false;
                }
                cur += c;
                ignore_space = 0;
            }
        }
        //return o;
    }

    std::string allToString(bool narrow = true) const
    {
        return to_string(narrow, 0);
    }
};
