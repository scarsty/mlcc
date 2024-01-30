#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace INIReader
{

struct KeyType
{
    std::string key, value;
};

template <class COM_METHOD>
class INIReader_t
{
public:
    template <typename T>
    struct Record
    {
        std::string key;
        T value;
    };
    template <typename T>
    struct ListWithIndex : std::list<Record<T>>
    {
        std::unordered_map<std::string, T*> index;
        T& operator[](const std::string& key)
        {
            auto key1 = COM_METHOD()(key);
            auto it = index.find(key1);
            if (it != index.end())
            {
                return *it->second;
            }
            else
            {
                std::list<Record<T>>::push_back({ key, T() });
                T* p = &std::list<Record<T>>::back().value;
                index.insert({ key1, p });
                return *p;
            }
        }
        const T& at(const std::string& key) const
        {
            return *index.at(COM_METHOD()(key));
        }
        int count(const std::string& key) const
        {
            return index.count(COM_METHOD()(key));
        }
        void erase(const std::string& key)
        {
            auto& it = (*this)[key];
            std::list<Record<T>>::remove_if([&](Record<KeyType1>& sec)
                {
                    return &sec.value == &it;
                });
            index.erase(COM_METHOD()(key));
        }
    };

public:
    struct KeyType1
    {
    public:
        std::string value;
        ListWithIndex<KeyType1> sections;
        std::string other;

    public:
        KeyType1() {}
        KeyType1(const std::string& value, const std::string& other = "") : value(value), other(other) {}
        KeyType1(const char* value) : value(value) {}
        template <typename T>
        KeyType1(const T& value) : value(std::to_string(value)) {}

        int toInt() const
        {
            return atoi(value.c_str());
        }
        double toDouble() const
        {
            return atof(value.c_str());
        }
        const std::string& toString() const
        {
            return value;
        }
        KeyType1& operator[](const std::string& key)
        {
            return sections[key];
        }
        const KeyType1& operator[](const std::string& key) const
        {
            return sections.at(key);
        }
        const int count(const std::string& key) const
        {
            return sections.count(key);
        }
        void erase(const std::string& key)
        {
            sections.erase(key);
        }
        bool isKey() const
        {
            return sections.size() == 0;
        }
        std::vector<std::string> getAllSections() const
        {
            std::vector<std::string> ret;
            for (auto& value : sections)
            {
                if (!value.value.isKey())
                {
                    ret.push_back(value.key);
                }
            }
            return ret;
        }
        std::vector<std::string> getAllKeys() const
        {
            std::vector<std::string> ret;
            for (auto& value : sections)
            {
                if (value.value.isKey())
                {
                    ret.push_back(value.key);
                }
            }
            return ret;
        }
        void addWithoutIndex(const KeyType1& value)
        {
            sections.push_back({ "", value });
        }
        std::string allToString(int layer = 1, bool show_other = true, const std::string& line_break = "\n") const    //ignore the value of first layer
        {
            std::string str;
            for (auto& sec : sections)
            {
                if (sec.value.sections.size() == 0)
                {
                    if (sec.key.empty())
                    {
                        if (show_other)
                        {
                            str += sec.value.other;
                        }
                    }
                    else
                    {
                        str += sec.key;
                        str += " = ";
                        str += dealValue(sec.value.value);
                        if (show_other)
                        {
                            str += sec.value.other;
                        }
                    }
                    str += line_break;
                }
            }
            for (auto& sec : sections)
            {
                if (sec.value.sections.size() > 0)
                {
                    str += std::string(layer, '[');
                    str += sec.key;
                    str += std::string(layer, ']');
                    str += line_break;
                    str += sec.value.allToString(layer + 1, show_other, line_break);
                }
            }
            return str;
        }
    };

private:
    KeyType1 keys;

#ifdef _WIN32
    std::string line_break_ = "\r\n";
#else
    std::string line_break_ = "\n";
#endif
    int error_ = 0;
    bool bom_ = false;

    //return value: the key has existed, 0 means it is a new key
    int valueHandler(KeyType1& keytype, const std::string& section, const std::string& key, const std::string& value, const std::string& other = "")
    {
        if (key.empty())
        {
            keytype[section].addWithoutIndex(KeyType1(value, other));    //comment or others
        }
        else
        {
            keytype[section][key] = KeyType1(value, other);
        }
        return 0;
    }

private:
    static std::string dealValue(const std::string& str)
    {
        if (str.find_first_of(";#\n\r") != std::string::npos)
        {
            return "\"" + str + "\"";
        }
        return str;
    }

public:
    INIReader_t()
    {
    }

    KeyType1& operator[](const std::string& key)
    {
        return keys[key];
    }

    // parse a given filename
    int loadFile(const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp)
        {
            //fprintf(stderr, "Cannot open file %s\n", filename.c_str());
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, 0);
        std::string str;
        str.resize(length, '\0');
        if (fread((void*)str.c_str(), 1, length, fp) < length)
        {
            //fprintf(stderr, "Read file %s unfinished\n", filename.c_str());
            return 1;
        }
        fclose(fp);
        loadString(str);
        return 0;
    }

    // parse an ini string
    void loadString(const std::string& content)
    {
        line_break_ = "\n";
        int pos = content.find(line_break_, 1);
        if (pos != std::string::npos && pos < content.size())
        {
            if (content[pos - 1] == '\r')
            {
                line_break_ = "\r\n";
            }
            else if (content[pos + 1] == '\r')
            {
                line_break_ = "\n\r";
            }
        }
        error_ = ini_parse_content(content);
    }
    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, or -1 on file open error.
    int parseError() const
    {
        return error_;
    }

    // Get a string value from INI file, returning default_value if not found.
    std::string getString(const std::string& section, const std::string& key, const std::string& default_value = "") const
    {
        auto section1 = (section);
        if (keys.count(section1) == 0)
        {
            return default_value;
        }
        auto key1 = (key);
        if (keys[section1].count(key1) == 0)
        {
            return default_value;
        }
        return keys[section1][key1].value;
    }

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    int getInt(const std::string& section, const std::string& key, int default_value = 0) const
    {
        auto valstr = getString(section, key, "");
        const char* value = valstr.c_str();
        char* end;
        // This parses "1234" (decimal) and also "0x4D2" (hex)
        int n = strtol(value, &end, 0);
        return end > value ? n : default_value;
    }

    // Get a real (floating point double) value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtod().
    double getReal(const std::string& section, const std::string& key, double default_value = 0.0) const
    {
        auto valstr = getString(section, key, "");
        const char* value = valstr.c_str();
        char* end;
        double n = strtod(value, &end);
        return end > value ? n : default_value;
    }

    // Get a boolean value from INI file, returning default_value if not found or if
    // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
    // and valid false values are "false", "no", "off", "0" (not case sensitive).
    bool getBoolean(const std::string& section, const std::string& key, bool default_value = false) const
    {
        auto valstr = getString(section, key, "");
        // Convert to lower case to make string comparisons case-insensitive
        std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
        if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        {
            return true;
        }
        else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        {
            return false;
        }
        else
        {
            return getReal(section, key, default_value);
        }
    }
    template <typename T>
    T get(const std::string& section, const std::string& key, T default_value = T(0)) const
    {
        auto v = stringTo<T>(getString(section, key, std::to_string(default_value)));
        return v;
    }
    std::vector<std::string> getStringVector(const std::string& section, const std::string& key, const std::string& split_chars = ",", const std::vector<std::string>& default_v = {}) const
    {
        auto v = splitString(getString(section, key), split_chars, true);
        if (v.empty()) { return default_v; }
        return v;
    }
    template <typename T>
    std::vector<T> getVector(const std::string& section, const std::string& key, const std::string& split_chars = ",", const std::vector<T>& default_v = {}) const
    {
        auto v = stringVectorToVector<T>(getStringVector(section, key, split_chars));
        if (v.empty()) { return default_v; }
        return v;
    }
    template <>
    std::vector<std::string> getVector(const std::string& section, const std::string& key, const std::string& split_chars, const std::vector<std::string>& default_v) const
    {
        auto v = splitString(getString(section, key), split_chars, true);
        if (v.empty()) { return default_v; }
        return v;
    }
    //check one section exist or not
    int hasSection(const std::string& section) const
    {
        return keys.count(section);
    }

    //check one section and one key exist or not
    int hasKey(const std::string& section, const std::string& key) const
    {
        if (keys.count(section) == 0)
        {
            return 0;
        }
        if (keys[section].count(key) == 0)
        {
            return 0;
        }
        return 1;
    }

    std::vector<std::string> getAllSections() const
    {
        std::vector<std::string> ret;
        for (auto& value : keys.sections)
        {
            ret.push_back(value.key);
        }
        return ret;
    }

    std::vector<std::string> getAllKeys(const std::string& section) const
    {
        std::vector<std::string> ret;
        if (keys.count(section) == 0)
        {
            return ret;
        }
        auto& sec = keys[section].sections;
        for (auto& kv : sec)
        {
            if (!kv.key.empty())
            {
                ret.push_back(kv.key);
            }
        }
        return ret;
    }

    std::vector<KeyType> getAllKeyValues(const std::string& section) const
    {
        std::vector<KeyType> ret;
        if (keys.count(section) == 0)
        {
            return ret;
        }
        auto& sec = keys[section].sections;
        for (auto& kv : sec)
        {
            if (!kv.key.empty())
            {
                ret.push_back({ kv.key, kv.value.value });
            }
        }
        return ret;
    }

    void setKey(const std::string& section, const std::string& key, const std::string& value)
    {
        valueHandler(keys, section, key, value);
    }

    void eraseKey(const std::string& section, const std::string& key)
    {
        keys[section].erase(key);
    }

    void eraseSection(const std::string& section)
    {
        keys.erase(section);
    }

    void clear()
    {
        keys.sections.clear();
        keys.sections.index.clear();
    }

private:
    int ini_parse_content(const std::string& content)
    {
        /* Return pointer to first non-whitespace char in given string */
        auto lskip = [](std::string& s) -> void
        {
            auto pre = s.find_first_not_of(" ");
            if (pre != std::string::npos)
            {
                s = s.substr(pre);
            }
        };

        /* Strip whitespace chars off end of given string */
        auto rstrip = [](std::string& s, size_t& suf) -> void
        {
            auto pos = s.find_last_not_of(" ");
            if (pos != std::string::npos)
            {
                suf = s.size() - pos - 1;
                s.resize(pos + 1);
            }
            else
            {
                suf = 0;
            }
        };

        /* Uses a fair bit of stack (use heap instead if you need to) */
        std::string section = "";
        int error = 0;
        /* Scan all lines */
        size_t i = 0;
        if (content.size() >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF)
        {
            i = 3;
            bom_ = true;
        }
        bool new_line = true;

        int status = 0;    //0: new line, 1: comment, 2: section, 3: key, 4: value
        std::string str;
        std::vector<KeyType1*> stack = { &keys };
        while (i < content.size())
        {
            auto& c = content[i];

            if (c == '\r') { i++; }
            else if (!new_line && c == '\n')
            {
                i++;
                new_line = true;
            }
            else if (new_line && c == '\n')
            {
                new_line = true;
                valueHandler(*stack.back(), section, "", "", "");
                i++;
            }
            else if (new_line && c == '[')
            {
                auto square_count = content.find_first_not_of("[", i + 1) - i;
                auto end = content.find(std::string(square_count, ']'), i + square_count);
                if (square_count > stack.size())
                {
                    stack.push_back(&stack.back()->sections[section]);
                }
                else if (square_count < stack.size())
                {
                    if (square_count < 1) { square_count = 1; }
                    stack.resize(square_count);
                }
                if (end != std::string::npos)
                {
                    section = content.substr(i + square_count, end - i - square_count);    //found a new section
                }
                i = end;
                new_line = false;
            }
            else if (new_line && c != ' ')
            {
                new_line = false;
                auto end = content.find_first_of("=", i + 1);
                if (end != std::string::npos)
                {
                    auto end2 = content.find_first_of(";#\n\r\'\"", i);
                    if (end2 < end)
                    {
                        auto endline = content.find_first_of("\n\r", i);
                        if (endline != std::string::npos)
                        {
                            std::string o = content.substr(i, endline - i);
                            valueHandler(*stack.back(), section, "", "", o);
                            i = endline;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    else
                    {
                        auto key = content.substr(i, end - i);    //found a new key
                        size_t suf;
                        rstrip(key, suf);
                        //if find a key, search the value from the next char of '=', considering quote and new line
                        size_t i1 = end + 1;
                        int quote = 0;    //0: no quote, 1: ', 2: "
                        std::string v;
                        size_t o_begin = std::string::npos;
                        bool begin = true;
                        bool end = false;
                        while (i1 < content.size())
                        {
                            auto& c1 = content[i1];
                            if (begin && c1 == ' ')
                            {
                                i1++;
                                continue;
                            }
                            if (begin && c1 != ' ')
                            {
                                begin = false;
                            }
                            if (!begin)
                            {
                                if (!end)
                                {
                                    if (quote == 0 && c1 == '\'') { quote = 1; }
                                    else if (quote == 0 && c1 == '\"') { quote = 2; }
                                    else if (quote == 1 && c1 == '\'') { quote = 0; }
                                    else if (quote == 2 && c1 == '\"') { quote = 0; }
                                }
                                if (quote == 0)
                                {
                                    if (c1 == '\r' || c1 == '\n')
                                    {
                                        i = i1;
                                        break;
                                    }
                                    if (c1 == '#' || c1 == ';')
                                    {
                                        o_begin = i1;
                                        end = true;
                                    }
                                    if (!end)
                                    {
                                        v += c1;
                                    }
                                }
                                else
                                {
                                    v += c1;
                                }
                            }
                            i1++;
                        }
                        if (o_begin == std::string::npos)
                        {
                            o_begin = i1;
                        }
                        //remove the last space of v
                        rstrip(v, suf);
                        if (suf)
                        {
                            o_begin -= suf;
                        }
                        if (v.size() >= 2
                            && (v.front() == '\'' && v.back() == '\'' || v.front() == '\"' && v.back() == '\"'))
                        {
                            v = v.substr(1, v.size() - 2);
                        }
                        valueHandler(*stack.back(), section, key, v, content.substr(o_begin, i1 - o_begin));
                        i = i1;
                    }
                }
            }
            else
            {
                i++;
            }
        }
        return 0;
    }

    static std::vector<std::string> splitString(std::string str, std::string pattern, bool ignore_psspace)
    {
        std::string::size_type pos;
        std::vector<std::string> result;
        if (str.empty())
        {
            return result;
        }
        if (pattern.empty())
        {
            pattern = ",;| ";
        }
        str += pattern[0];    //expand string to find the last one
        bool have_space = pattern.find(" ") != std::string::npos;
        int size = str.size();
        for (int i = 0; i < size; i++)
        {
            if (have_space)
            {
                //treat continuous space as one, when space is in pattern
                while (str[i] == ' ')
                {
                    i++;
                }
            }
            pos = str.find_first_of(pattern, i);
            if (pos < size)
            {
                std::string s = str.substr(i, pos - i);
                if (ignore_psspace)
                {
                    auto pre = s.find_first_not_of(" ");
                    auto suf = s.find_last_not_of(" ");
                    if (pre != std::string::npos && suf != std::string::npos)
                    {
                        s = s.substr(pre, suf - pre + 1);
                    }
                }
                result.push_back(s);
                i = pos;
            }
        }
        return result;
    }
    //only support int, float, double
    template <typename T>
    static T stringTo(const std::string& s)
    {
        double v = atof(s.c_str());
        return T(v);
    }
    template <typename T>
    static std::vector<T> stringVectorToVector(const std::vector<std::string>& v0)
    {
        std::vector<T> v;
        for (auto& s : v0)
        {
            v.push_back(stringTo<T>(s));
        }
        return v;
    }

public:
    //write modified file
    int saveFile(const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp)
        {
            auto content = toString();
            int length = content.length();
            fwrite(content.c_str(), length, 1, fp);
            fclose(fp);
            return 0;
        }
        return 1;
    }

    //make a string with trying to keep the original style
    std::string toString(bool comment = true) const
    {
        std::string content;
        if (bom_)
        {
            content += "\xEF\xBB\xBF";
        }
        bool first = true;
        content += keys.allToString(1, comment, line_break_);
        if (content.size() >= line_break_.size())
        {
            content.resize(content.size() - line_break_.size());
        }
        return content;
    }

    //a pure string without comments or blank lines
    std::string toPureString() const
    {
        return toString(false);
    }
};

struct CaseSensitivity
{
    std::string operator()(const std::string& l) const
    {
        return l;
    }
};

struct CaseInsensitivity
{
    std::string operator()(const std::string& l) const
    {
        auto l1 = l;
        std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
        return l1;
    }
};

struct NoUnderline
{
    std::string operator()(const std::string& l) const
    {
        auto l1 = l;
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
        std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
        return l1;
    }
};
};    //namespace INIReader

using INIReaderNormal = INIReader::INIReader_t<INIReader::CaseInsensitivity>;
using INIReaderNoUnderline = INIReader::INIReader_t<INIReader::NoUnderline>;
