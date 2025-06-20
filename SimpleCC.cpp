
#include "SimpleCC.h"
#include "filefunc.h"
#include "strfunc.h"
#include <functional>
#include <print>

struct Words
{
    int length;           //替换前的字数
    std::string value;    //替换后的字
};

std::string SimpleCC::conv(const std::string& src)
{
    if (root_.children.empty())
    {
        return src;    // No translations available, return original string
    }

    auto translate = [&](const std::string& str, std::string& str1, int begin, int& end)
    {
        auto* t = &root_;
        end = begin;
        int i = 0, j = 0;
        std::vector<Words> strs;
        while (end < str.size())
        {
            int len = utf8length(str[end]);
            auto key = str.substr(end, len);
            i++;
            if (t->children.contains(key))
            {
                end += len;
                //递归查找词组
                if (!t->children[key].value.empty())
                {
                    strs.push_back({ end - begin, t->children[key].value });
                }
                t = &t->children[key];
            }
            else
            {
                break;
            }
        }
        if (strs.empty())
        {
            int len = utf8length(str[begin]);
            str1 = str.substr(begin, len);
            end = begin + len;
        }
        else
        {
            str1 = strs.back().value;
            end = begin + strs.back().length;
        }
    };

    std::string dst;
    int i = 0;
    while (i < src.size())
    {
        int i1 = i;
        std::string str1;
        translate(src, str1, i, i1);
        if (str1.empty())
        {
            dst += src.substr(i, i1 - i);
        }
        else
        {
            dst += str1;
        }
        i = i1;
    }
    return dst;
}

int SimpleCC::init(std::vector<std::string> files)
{
    for (const auto& file : files)
    {
        if (!filefunc::fileExist(file))
        {
            continue;    // Skip files that do not exist
        }
        auto str = filefunc::readFileToString(file);
        int i = 0;
        //must be utf-8 encoded chinese text
        while (i < str.size())
        {
            uint8_t code = str[i];
            if ((code & 0xF0) == 0xE0)    // 3-byte sequence
            {
                auto pos_space = str.find('\t', i);
                if (pos_space == std::string::npos)
                {
                    break;
                }
                std::string key = str.substr(i, pos_space - i);
                auto pos_line = str.find_first_of("\r\n", pos_space + 1);
                if (pos_line == std::string::npos)
                {
                    break;
                }
                std::string value = str.substr(pos_space + 1, pos_line - pos_space - 1);
                if (value.contains(' '))
                {
                    value = value.substr(0, value.find(' '));
                }
                write(key, value);
                i = pos_line + 1;
            }
            else
            {
                i++;
            }
        }
    }

    return 0;
}

void SimpleCC::write(const std::string& key, const std::string& value)
{
    //if (value.size() <= 3)
    //{
    //    single_[key] = value;
    //    return;
    //}
    std::function<void(const std::string&, const std::string&, Tire&)> write_key = [&](const std::string& key, const std::string& value, Tire& t)
    {
        if (key.size() <= 3)
        {
            t.children[key].value = value;
        }
        else
        {
            write_key(key.substr(3), value, t.children[key.substr(0, 3)]);
        }
    };
    write_key(key, value, root_);
}

int SimpleCC::utf8length(const unsigned char c)
{
    if (c < 0x80)
    {
        return 1;    // 1 byte character
    }
    else if ((c & 0xE0) == 0xC0)
    {
        return 2;    // 2 byte character
    }
    else if ((c & 0xF0) == 0xE0)
    {
        return 3;    // 3 byte character
    }
    else if ((c & 0xF8) == 0xF0)
    {
        return 4;    // 4 byte character
    }
    return 1;    // Default to 1 byte if not recognized
}
