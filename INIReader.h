// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

#pragma once

//Nonzero to allow multi-line value parsing, in the style of Python's
//configparser. If allowed, ini_parse() will call the handler with the same
//name for each subsequent line parsed.
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

//Nonzero to allow inline comments (with valid inline comment characters
//specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
//Python 3.2+ configparser behaviour.
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";#"
#endif

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)

class INIReader
{
public:
    struct KeyType
    {
        std::string key, value, other;
    };
    struct Section
    {
        std::string name;
        std::list<KeyType> keys;
    };

private:
    std::list<Section> sections_;

    struct Index
    {
        Section* ps = nullptr;
        std::map<std::string, KeyType*> pks;
    };

    std::function<std::string(const std::string&)> compare_section_;
    std::function<std::string(const std::string&)> compare_key_;

    std::map<std::string, Index> index_section_;

#ifdef _WIN32
    std::string line_break_ = "\r\n";
#else
    std::string line_break_ = "\n";
#endif
    int error_ = 0;
    std::vector<std::string> lines_;    //lines of the files

    //return value: the key has existed, 0 means it is a new key
    int valueHandler(const std::string& section, const std::string& key, const std::string& value, const std::string& other = "")
    {
        Section* ps = nullptr;
        auto section1 = compare_section_(section);
        auto key1 = compare_key_(key);
        if (index_section_.count(section1) == 0)
        {
            sections_.push_back({ section, {} });
            index_section_[section1] = { &sections_.back(), {} };
        }
        ps = index_section_[section1].ps;
        if (key1 == "")
        {
            ps->keys.push_back({ key, value, other });
        }
        else if (ps)
        {
            if (index_section_[section1].pks.count(key1) == 0)
            {
                ps->keys.push_back({ key, value, other });
                index_section_[section1].pks[key1] = &ps->keys.back();
            }
            else
            {
                *index_section_[section1].pks[key1] = { key, value, other };
            }
        }
        return 0;
    }

private:
    static std::string dealValue(const std::string str)
    {
        std::string str1;
        if (str.find_first_of(INI_INLINE_COMMENT_PREFIXES) != std::string::npos)
        {
            return "\"" + str + "\"";
        }
        return str;
    }

public:
    INIReader()
    {
        compare_section_ = [](const std::string& l)
        {
            return l;
        };
        compare_key_ = [](const std::string& l)
        {
            return l;
        };
    }

    void setCompareSection(std::function<std::string(const std::string&)> com)
    {
        compare_section_ = com;
    }
    void setCompareKey(std::function<std::string(const std::string&)> com)
    {
        compare_key_ = com;
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
        auto section1 = compare_section_(section);
        if (index_section_.count(section1) == 0)
        {
            return default_value;
        }
        auto key1 = compare_key_(key);
        if (index_section_.at(section1).pks.count(key1) == 0)
        {
            return default_value;
        }
        return index_section_.at(section1).pks.at(key1)->value;
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
    std::vector<double> getRealVector(const std::string& section, const std::string& key, const std::string& split_chars = ",", const std::vector<double>& default_v = {}) const
    {
        auto v = stringVectorToVector<double>(getStringVector(section, key, split_chars));
        if (v.empty()) { return default_v; }
        return v;
    }
    std::vector<int> getIntVector(const std::string& section, const std::string& key, const std::string& split_chars = ",", const std::vector<int>& default_v = {}) const
    {
        auto v = stringVectorToVector<int>(getStringVector(section, key, split_chars));
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
    //check one section exist or not
    int hasSection(const std::string& section) const
    {
        return index_section_.count(compare_section_(section));
    }

    //check one section and one key exist or not
    int hasKey(const std::string& section, const std::string& key) const
    {
        if (index_section_.count(compare_section_(section)) == 0)
        {
            return 0;
        }
        if (index_section_.at(compare_section_(section)).pks.count(compare_key_(key)) == 0)
        {
            return 0;
        }
        return 1;
    }

    std::vector<std::string> getAllSections() const
    {
        std::vector<std::string> ret;
        for (auto& value : sections_)
        {
            ret.push_back(value.name);
        }
        return ret;
    }

    std::vector<std::string> getAllKeys(const std::string& section) const
    {
        std::vector<std::string> ret;
        if (index_section_.count(compare_section_(section)) == 0)
        {
            return ret;
        }
        auto& sec = index_section_.at(compare_section_(section));
        for (auto& kv : sec.pks)
        {
            ret.push_back(kv.first);
        }
        return ret;
    }

    std::vector<KeyType> getAllKeyValues(const std::string& section) const
    {
        std::vector<KeyType> ret;
        if (index_section_.count(compare_section_(section)) == 0)
        {
            return ret;
        }
        auto ps = index_section_.at(compare_section_(section)).ps;
        for (auto& kv : ps->keys)
        {
            ret.push_back(kv);
        }
        return ret;
    }

    void setKey(const std::string& section, const std::string& key, const std::string& value)
    {
        valueHandler(section, key, value);
    }

    void eraseKey(const std::string& section, const std::string& key)
    {
        auto& it = index_section_[compare_section_(section)];
        auto& pkey = it.pks[compare_key_(key)];
        it.ps->keys.remove_if([=](KeyType& value)
            {
                return &value == pkey;
            });
        it.pks.erase(compare_key_(key));
    }

    void eraseSection(const std::string& section)
    {
        auto& it = index_section_[compare_section_(section)];
        sections_.remove_if([&](Section& sec)
            {
                return &sec == it.ps;
            });
        index_section_.erase(compare_section_(section));
    }

    void clear()
    {
        sections_.clear();
    }

    void clearAll()
    {
        lines_.clear();
        sections_.clear();
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
                s = s.substr(0, pos + 1);
            }
            else
            {
                suf = 0;
            }
        };

        /* Uses a fair bit of stack (use heap instead if you need to) */
        std::string section = "";
        std::string prev_key = "";
        int error = 0;
        /* Scan all lines */
        size_t i = 0;
        if (content.size() >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF)
        {
            i = 3;
        }
        bool new_line = true;

        int status = 0;    //0: new line, 1: comment, 2: section, 3: key, 4: value
        std::string str;
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
                valueHandler(section, "", "", "");
                i++;
            }
            else if (new_line && c == '[')
            {
                auto end = content.find_first_of("]", i + 1);
                if (end != std::string::npos)
                {
                    section = content.substr(i + 1, end - i - 1);    //found a new section
                    prev_key = "";
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
                            valueHandler(section, "", "", o);
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
                        std::string v, o;
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
                                        end = true;
                                    }
                                    if (!end)
                                    {
                                        v += c1;
                                    }
                                    else
                                    {
                                        o += c1;
                                    }
                                }
                                else
                                {
                                    v += c1;
                                }
                            }
                            i1++;
                        }
                        //remove the last space of v
                        rstrip(v, suf);
                        if (suf)
                        {
                            o = std::string(suf, ' ') + o;
                        }
                        if (v.front() == '\'' && v.back() == '\''
                            || v.front() == '\"' && v.back() == '\"')
                        {
                            v = v.substr(1, v.size() - 2);
                        }
                        valueHandler(section, key, v, o);
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
        str += pattern[0];    //扩展字符串以方便操作
        bool have_space = pattern.find(" ") != std::string::npos;
        int size = str.size();
        for (int i = 0; i < size; i++)
        {
            if (have_space)
            {
                //当空格作为分隔符时，连续空格视为一个
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
    std::string toString()
    {
        std::string content;
        bool first = true;
        for (auto& sec : sections_)
        {
            content += "[" + sec.name + "]" + line_break_;
            if (first && sec.name.empty())
            {
                first = false;
                content = "";
            }
            for (auto& key : sec.keys)
            {
                if (key.key.empty())
                {
                    content += key.other + line_break_;
                }
                else
                {
                    content += key.key + " = " + dealValue(key.value) + key.other + line_break_;
                }
            }
        }
        return content;
    }

    //a pure string without comments or blank lines
    std::string toPureString() const
    {
        std::string content;
        bool first = true;
        for (auto& sec : sections_)
        {
            if (first && sec.name.empty())
            {
                first = false;
            }
            else
            {
                content += "[" + sec.name + "]" + line_break_;
            }
            for (auto& key : sec.keys)
            {
                if (!key.key.empty())
                {
                    content += key.key + " = " + dealValue(key.value) + line_break_;
                }
            }
        }
        return content;
    }
};

// The most widely used
class INIReaderNormal : public INIReader
{
public:
    INIReaderNormal()
    {
        auto compare_case_insensitivity = [](const std::string& l)
        {
            auto l1 = l;
            std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
            return l1;
        };
        setCompareSection(compare_case_insensitivity);
        setCompareKey(compare_case_insensitivity);
    }
};
class INIReaderNoUnderline : public INIReader
{
public:
    INIReaderNoUnderline()
    {
        setCompareSection([](const std::string& l)
            {
                auto l1 = l;
                std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
                return l1;
            });
        setCompareKey([](const std::string& l)
            {
                auto l1 = l;
                auto erase = [](std::string& s, const std::string& oldstring)
                {
                    auto pos = s.find(oldstring);
                    while (pos != std::string::npos)
                    {
                        s.erase(pos, oldstring.length());
                        pos = s.find(oldstring, pos);
                    }
                };
                erase(l1, "_");
                std::transform(l1.begin(), l1.end(), l1.begin(), ::tolower);
                return l1;
            });
    }
};
