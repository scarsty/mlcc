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

//Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
//the file. See http://code.google.com/p/inih/issues/detail?id=21
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
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

//Stop parsing on first error (default is to keep parsing).
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)

class INIReader
{
private:
    enum
    {
        READ = 0,
        WRITE = 1,
    };
    struct KeyType
    {
        std::string name, value;
    };

    template <typename T>
    struct BaseType
    {
        std::string name;
        std::list<T> records;
        std::map<std::string, typename std::list<T>::iterator> map_iter;
        std::function<std::string(const std::string&)> compare_name;

        T& operator[](const std::string& str)
        {
            auto str1 = compare_name(str);
            if (map_iter.count(str1))
            {
                return *map_iter[str1];
            }
            records.emplace_back();
            records.back().name = str;
            map_iter[str1] = std::prev(records.end());
            return records.back();
        }
        const T& at(const std::string& str) const
        {
            auto str1 = compare_name(str);
            if (map_iter.count(str1))
            {
                return *(map_iter.at(str1));
            }
            return T();
        }
        size_t count(const std::string& str) const
        {
            auto str1 = compare_name(str);
            return map_iter.count(str1);
        }
        void erase(const std::string& str)
        {
            auto str1 = compare_name(str);
            if (map_iter.count(str1))
            {
                records.erase(map_iter[str1]);
                map_iter.erase(str1);
            }
            return;
        }
        void clear()
        {
            records.clear();
            map_iter.clear();
        }
        void rebuildMap()
        {
            map_iter.clear();
            for (auto it = records.begin(); it != records.end(); it++)
            {
                map_iter[compare_name(it->name)] = it;
            }
        }
        BaseType() = default;
        BaseType(const BaseType& st)
        {
            name = st.name;
            records = st.records;
            compare_name = st.compare_name;
            rebuildMap();
        }
        BaseType& operator=(const BaseType& st)
        {
            if (this == &st) { return *this; }
            name = st.name;
            records = st.records;
            compare_name = st.compare_name;
            rebuildMap();
            return *this;
        }
        BaseType(BaseType&& st) noexcept = default;
        BaseType& operator=(BaseType&& st) noexcept = default;
    };
    using SectionType = BaseType<KeyType>;
    struct FileType : public BaseType<SectionType>
    {
        std::function<std::string(const std::string&)> compare_key;
        SectionType& operator[](const std::string& str)
        {
            auto str1 = compare_name(str);
            if (map_iter.count(str1))
            {
                return *map_iter[str1];
            }
            records.emplace_back();
            records.back().name = str;
            records.back().compare_name = compare_key;
            map_iter[str1] = std::prev(records.end());
            return records.back();
        }
        const SectionType& at(const std::string& str) const
        {
            auto str1 = compare_name(str);
            if (map_iter.count(str1))
            {
                return *(map_iter.at(str1));
            }
            return SectionType();
        }
    };
#ifdef _WIN32
    std::string line_break_ = "\r\n";
#else
    std::string line_break_ = "\n";
#endif
    int error_ = 0;
    std::vector<std::string> lines_;    //lines of the files, sections the lines belong to
    FileType sections_;
    mutable std::mutex mutex_;

    //return value: the key has existed, 0 means it is a new key
    int valueHandler(const std::string& section, const std::string& key, const std::string& value)
    {
        int ret = sections_[section].count(key);
        sections_[section][key].value = value;
        return ret;
    }

public:
    INIReader()
    {
        sections_.compare_name = [](const std::string& l)
        {
            return l;
        };
        sections_.compare_key = [](const std::string& l)
        {
            return l;
        };
    }

    const std::string& operator()(const std::string& section, const std::string& key) const
    {
        return sections_.at(section).at(key).value;
    }

    std::string& operator()(const std::string& section, const std::string& key)
    {
        return sections_[section][key].value;
    }

    void setCompareSection(std::function<std::string(const std::string&)> com)
    {
        sections_.compare_name = com;
    }
    void setCompareKey(std::function<std::string(const std::string&)> com)
    {
        sections_.compare_key = com;
        for (auto& k : sections_.records)
        {
            k.compare_name = com;
        }
    }

    // parse a given filename
    void loadFile(const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp)
        {
            //fprintf(stderr, "Cannot open file %s\n", filename.c_str());
            return;
        }
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, 0);
        std::string str;
        str.resize(length, '\0');
        if (fread((void*)str.c_str(), 1, length, fp) < length)
        {
            //fprintf(stderr, "Read file %s unfinished\n", filename.c_str());
        }
        fclose(fp);
        loadString(str, true);
    }

    // parse an ini string
    void loadString(const std::string& content, bool clear_style = true)
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
        error_ = ini_parse_content(content, clear_style);
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
        std::lock_guard<std::mutex> lock(mutex_);
        if (sections_.count(section) == 0)
        {
            return default_value;
        }
        if (sections_.at(section).count(key) > 0)
        {
            return sections_.at(section).at(key).value;
        }
        else
        {
            return default_value;
        }
    }

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    int getInt(const std::string& section, const std::string& key, long default_value = 0) const
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

    //check one section exist or not
    int hasSection(const std::string& section) const
    {
        return sections_.count(section);
    }

    //check one section and one key exist or not
    int hasKey(const std::string& section, const std::string& key) const
    {
        return sections_.count(section) > 0 ? sections_.at(section).count(key) : 0;
    }

    std::vector<std::string> getAllSections() const
    {
        std::vector<std::string> ret;
        for (auto& value : sections_.records)
        {
            ret.push_back(value.name);
        }
        return ret;
    }

    std::vector<std::string> getAllKeys(const std::string& section) const
    {
        std::vector<std::string> ret;
        if (sections_.count(section) == 0)
        {
            return ret;
        }
        for (auto& kv : sections_.at(section).records)
        {
            ret.push_back(kv.name);
        }
        return ret;
    }

    std::vector<KeyType> getAllKeyValues(const std::string& section) const
    {
        std::vector<KeyType> ret;
        if (sections_.count(section) == 0)
        {
            return ret;
        }
        for (auto& kv : sections_.at(section).records)
        {
            ret.push_back(kv);
        }
        return ret;
    }

    void setKey(const std::string& section, const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        valueHandler(section, key, value);
    }

    void eraseKey(const std::string& section, const std::string& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sections_[section].erase(key);
    }

    void eraseSection(const std::string& section)
    {
        sections_.erase(section);
    }

    void print() const
    {
        for (auto& skv : sections_.records)
        {
            fprintf(stdout, "[%s]\n", skv.name.c_str());
            for (auto& kv : skv.records)
            {
                fprintf(stdout, "%s = %s\n", kv.name.c_str(), kv.value.c_str());
            }
            fprintf(stdout, "\n");
        }
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
    int ini_parse_content(const std::string& content, bool clear_style)
    {
        //split the content into lines
        auto splitString = [](std::string str, std::string pattern)
        {
            std::string::size_type pos;
            std::vector<std::string> result;
            str += pattern[0];
            int size = str.size();
            char pre_pattern = '\0';
            for (int i = 0; i < size; i++)
            {
                pos = str.find_first_of(pattern, i);
                if (pos < size)
                {
                    std::string s = str.substr(i, pos - i);
                    if (s.empty() && pre_pattern != '\0' && pre_pattern != str[pos])    //a pair of different patterns are omitted,
                    {
                        pre_pattern = '\0';
                    }
                    else
                    {
                        result.push_back(s);
                        pre_pattern = str[pos];
                    }
                    i = pos;
                }
            }
            return result;
        };
        if (clear_style)
        {
            lines_ = splitString(content, "\n\r");
            return ini_parse_lines(lines_, READ);
        }
        else
        {
            auto lines = splitString(content, "\n\r");
            std::vector<std::string> lines_section;
            return ini_parse_lines(lines, READ);
        }
    }

    int ini_parse_lines(std::vector<std::string>& lines, int mode)
    {
        /* Return pointer to first non-whitespace char in given string. */
        auto lskip = [](const std::string& s) -> std::string
        {
            auto pre = s.find_first_not_of(" ");
            if (pre != std::string::npos)
            {
                return s.substr(pre);
            }
            else
            {
                return "";
            }
        };

        /* Strip whitespace chars off end of given string, in place. Return s. */
        auto rstrip = [](const std::string& s) -> std::string
        {
            auto suf = s.find_last_not_of(" ");
            if (suf != std::string::npos)
            {
                return s.substr(0, suf + 1);
            }
            else
            {
                return "";
            }
        };

        /* Uses a fair bit of stack (use heap instead if you need to) */
        std::string section = "";
        std::string prev_key = "";
        int lineno = 0;
        int error = 0;
        /* Scan all lines */
        auto it = lines.begin();
        while (it != lines.end())
        {
            lineno++;
            std::string& line = *it;
            /* Skip comment */
            if (lskip(line).find_first_of(INI_INLINE_COMMENT_PREFIXES) == 0)
            {
                it++;
                prev_key = "";
                continue;
            }
#if INI_ALLOW_BOM
            if (lineno == 1 && line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
            {
                line = line.substr(3);
            }
#endif
            line = rstrip(line);    //remove spaces on two sides

            if (line.size() < 2)    //a line should contains at least two letters such as "a=", "[]"
            {
            }
            else if (line[0] == ';' || line[0] == '#')
            {
                /* Per Python configparser, allow both ; and # comments at the start of a line */
            }
#if INI_ALLOW_MULTILINE
            else if (prev_key != "" && line[0] == ' ')
            {
                if (mode == READ)
                {
                    /* Non-blank line with leading whitespace, treat as continuation of previous name's value (as per Python config parser). */
                    std::string value = getString(section, prev_key, "") + line_break_ + rstrip(line);
                    valueHandler(section, prev_key, value);
                    if (error == 0)
                    {
                        //error = lineno;
                    }
                }
                else
                {
                    /* Ignore this line, WARRNING: comment will be lost! */
                    it = lines.erase(it);
                    continue;
                }
            }
#endif
            else if (line[0] == '[')
            {
                //if is writing and the previous section is not empty, insert it
                if (mode == WRITE && hasSection(section))
                {
                    for (auto& key : getAllKeys(section))
                    {
                        if (!key.empty())
                        {
                            std::string line = key + " = " + getString(section, key);
                            it = lines_.insert(it, line);
                        }
                    }
                    eraseSection(section);
                }
                /* A "[section]" line */
                auto end = line.find_first_of("]");
                if (end != std::string::npos)
                {
                    section = line.substr(1, end - 1);    //found a new section
                    prev_key = "";
                    if (mode == WRITE && !hasSection(section))
                    {
                        it = lines_.erase(it);
                        continue;
                    }
                }
                else if (error == 0)
                {
                    /* No ']' found on section line */
                    error = lineno;
                }
            }
            else
            {
                /* Not a comment, must be a name[=]value pair */
                auto assign_char = line.find_first_of("=");
                if (assign_char != std::string::npos)
                {
                    std::string key = rstrip(line.substr(0, assign_char));
                    std::string value = lskip(line.substr(assign_char + 1));
                    /* unsupport quote
                    int quote = 0;    //0: no quote, 1: single quote, 2: double quote
                    int quote_end_pos = value.size() - 1;
                    if (value.find_first_of("\'") == 0)
                    {
                        quote = 1;
                        quote_end_pos = value.find_first_of("\'", 1);
                    }
                    else if (value.find_first_of("\"") == 0)
                    {
                        quote = 2;
                        quote_end_pos = value.find_first_of("\"", 1);
                    }
                    if (quote)
                    {
                        if (quote_end_pos >= 0)
                        {
                            value = value.substr(1, quote_end_pos - 1);
                        }
                        else
                        {
                            value = value.substr(1);
                        }
                    }*/
                    std::string comment = "";
#if INI_ALLOW_INLINE_COMMENTS
                    //if (quote == 0)
                    {
                        int comment_pos = value.find_first_of(INI_INLINE_COMMENT_PREFIXES);
                        if (comment_pos != std::string::npos)
                        {
                            comment = value.substr(comment_pos);
                            value = value.substr(0, comment_pos);
                        }
                    }
#endif
                    auto blanks = value.substr(value.find_last_not_of(" ") + 1);
                    value = rstrip(value);

                    /* Valid name[=:]value pair found, call handler */
                    prev_key = key;

                    if (mode == READ)
                    {
                        int key_exist = valueHandler(section, key, value);
                        if (key_exist > 0)
                        {
                            //repeat defined, how to deal it?
                        }
                    }
                    else if (mode == WRITE)
                    {
                        if (hasKey(section, key))
                        {
                            auto value1 = getString(section, key, value);
                            if (value1 != value)
                            {
                                //rewrite the line
                                *it = key + " = " + value1 + blanks + comment;
                            }
                            eraseKey(section, key);
                        }
                        else
                        {
                            //key has been erased
                            it = lines.erase(it);
                            continue;
                        }
                    }
                    if (!error)
                    {
                        //error = lineno;
                    }
                }
                else if (error == 0)
                {
                    /* No '=' or ':' found on name[=:]value line */
                    error = lineno;
                    prev_key = "";
                }
            }
            it++;
        }
        //new keys for the last section
        if (mode == WRITE && hasSection(section))
        {
            for (auto& key : getAllKeys(section))
            {
                if (!key.empty())
                {
                    std::string line = key + " = " + getString(section, key);
                    it = lines_.insert(it, line);
                }
            }
            eraseSection(section);
        }
        return error;
    }

    void resetLines()
    {
        auto sections0 = sections_;
        //rescan the file to modify existing keys
        ini_parse_lines(lines_, WRITE);
        for (auto& section : getAllSections())
        {
            lines_.insert(lines_.end(), "[" + section + "]");
            for (auto& key : getAllKeys(section))
            {
                lines_.insert(lines_.end(), key + " = " + getString(section, key));
            }
        }
        sections_ = std::move(sections0);
    }

public:
    //write modified file
    void saveFile(const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp)
        {
            auto content = toString();
            int length = content.length();
            fwrite(content.c_str(), length, 1, fp);
            fclose(fp);
        }
    }

    //make a string with trying to keep the original style
    std::string toString()
    {
        resetLines();
        std::string content;
        for (int i = 0; i < lines_.size(); i++)
        {
            auto& line = lines_[i];
            content += line;
            if (i != lines_.size() - 1)
            {
                content += line_break_;
            }
        }
        return content;
    }

    //a pure string without comments or blank lines
    std::string toPureString() const
    {
        std::string content;
        for (auto& section : getAllSections())
        {
            content += "[" + section + "]" + line_break_;
            for (auto& key : sections_.at(section).records)
            {
                content += key.name + "=" + key.value + line_break_;
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
