#pragma once

#include "INIReader.h"
#include "strfunc.h"
#include "string.h"

struct INIReaderBin
{
private:
    INIReaderNormal ini_;
    const char* head_ = "CFG_BIN INI";
    const int head_size_ = 32;
    // The head is "CFG_BIN INI", 32 bytes is kept
    // The next 8 bytes is the length of the txt information
    // Next is the txt information. One value has two integers which are the differ from the beginning of content and the length
    // Next is the binary content
    // The beginning of the binary is 40+length of txt
public:
    int parse(const std::string& str)
    {
        if (str.size() > head_size_ + sizeof(uint64_t) && str.substr(0, 11) == head_)
        {
            uint64_t size_ini = 0;
            if (str.size() >= sizeof(uint64_t))
            {
                size_ini = *(uint64_t*)(str.data() + head_size_);
            }
            uint64_t begin = head_size_ + sizeof(uint64_t) + size_ini;
            INIReaderNormal assist;
            if (str.size() < begin)
            {
                return -1;
            }
            assist.loadString(str.substr(head_size_ + sizeof(uint64_t), size_ini));
            for (auto& section : assist.getAllSections())
            {
                for (auto& key : assist.getAllKeys(section))
                {
                    auto vec = assist.getVector<int>(section, key);
                    if (vec.size() == 2)
                    {
                        if (begin + vec[0] + vec[1] > str.size())
                        {
                            return -1;
                        }
                        ini_.setKey(section, key, str.substr(begin + vec[0], vec[1]));
                    }
                }
            }
            return 0;
        }
        return -1;
    }

    // The result may be not a text string.
    std::string to_string()
    {
        std::string str_content;
        INIReaderNormal assist;
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
        std::string result = head_;
        result.resize(head_size_ + sizeof(uint64_t));
        memcpy(&result[head_size_], &l, sizeof(uint64_t));
        result = result + str_ini + str_content;
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
