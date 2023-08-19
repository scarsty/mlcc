#pragma once
#include <map>
#include <string>

#include "strfunc.h"

// This format is used to store text or binary data in one file.
// Like XML, the name of the key is placed before and after the value. If some coincidence happen, just damn it.

struct LikeXML
{
private:
    std::map<std::string, std::string> values;

public:
    int parse(const std::string& str)
    {
        values.clear();
        size_t i = 0;
        int count = 0;
        while (i < str.size())
        {
            char c = str[i];
            if (c == '<')
            {
                auto i1 = str.find('>', i);
                if (i1 != std::string::npos)
                {
                    std::string key = str.substr(i + 1, i1 - i - 1);
                    auto i2 = str.find("</" + key + ">", i1);
                    if (i2 != std::string::npos)
                    {
                        std::string value = str.substr(i1 + 1, i2 - i1 - 1);
                        values[key] = value;
                        count++;
                        i = i2 + key.size() + 3;
                        continue;
                    }
                }
            }
            i++;
        }
        return 0;
    }

    // The result may be not a text string.
    std::string to_string()
    {
        std::string str;
        for (auto& kv : values)
        {
            str += "<" + kv.first + ">" + kv.second + "</" + kv.first + ">";
        }
        return str;
    }

    int save(const std::string& filename)
    {
        return strfunc::writeStringToFile(to_string(), filename);
    }

    int load(const std::string& filename)
    {
        return parse(strfunc::readStringFromFile(filename));
    }

    std::string get_value(const std::string& key)
    {
        if (values.count(key))
        {
            return values[key];
        }
        return "";
    }

    void set_value(const std::string& key, const std::string& value)
    {
        values[key] = value;
    }
};
