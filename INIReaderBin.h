#pragma once
#include <map>
#include <string>

#include "INIReader.h"
#include "strfunc.h"

// This format is used to store text or binary data in one file.
// Like XML, the name of the key is placed before and after the value. If some coincidence happen, just damn it.

struct INIReaderBin
{
private:
    INIReader ini_;
    std::map<std::string, std::string> values;

public:
    int parse(const std::string& str)
    {
        uint64_t size_ini = 0;
        if (str.size() >= sizeof(uint64_t))
        {
            size_ini = *(uint64_t*)str.data();
        }
        uint64_t begin = sizeof(uint64_t) + size_ini;
        INIReader assist;
        assist.loadString(str.substr(sizeof(uint64_t), size_ini));
        for (auto& section : assist.getAllSections())
        {
            for (auto& key : assist.getAllKeys(section))
            {
                auto vec = assist.getIntVector(section, key);
                if (vec.size() == 2)
                {
                    ini_.setKey(section, key, str.substr(begin + vec[0], vec[1]));
                }
            }
        }
        return 0;
    }

    // The result may be not a text string.
    std::string to_string()
    {
        std::string str_content;
        INIReader assist;
        for (auto& section : ini_.getAllSections())
        {
            for (auto& key : ini_.getAllKeys(section))
            {
                std::string str1 = ini_.getString(section, key);
                assist.setKey(section, key, std::to_string(str_content.size()) + "," + std::to_string(str1.size()));
                str_content += str1;
            }
        }
        auto str_ini = assist.toString();
        uint64_t l = str_ini.size();
        std::string result;
        result.resize(sizeof(uint64_t));
        memcpy(&result[0], &l, sizeof(uint64_t));
        result += str_ini + str_content;
        return result;
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
        return ini_.getString("", key);
    }

    void set_value(const std::string& key, const std::string& value)
    {
        ini_.setKey("", key, value);
    }
};
