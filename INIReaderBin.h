#pragma once

#include "INIReader.h"
#include "filefunc.h"
#include <cstdio>

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

    // File-based lazy loading state
    bool file_mode_ = false;
    std::string file_path_;
    uint64_t binary_begin_ = 0;
    INIReaderNormal file_index_;    // stores "offset,length" for each key as text
    FILE* fp_ = nullptr;

public:
    INIReaderBin() = default;
    INIReaderBin(const INIReaderBin&) = delete;
    INIReaderBin& operator=(const INIReaderBin&) = delete;

    ~INIReaderBin()
    {
        if (fp_) { fclose(fp_); fp_ = nullptr; }
    }

    int parse(const std::string& str)
    {
        if (str.size() > (size_t)head_size_ + sizeof(uint64_t) && str.substr(0, 11) == head_)
        {
            uint64_t size_ini = 0;
            if (str.size() >= (size_t)head_size_ + sizeof(uint64_t))
            {
                memcpy(&size_ini, str.data() + head_size_, sizeof(uint64_t));
            }
            uint64_t begin = (uint64_t)head_size_ + sizeof(uint64_t) + size_ini;
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
                    auto vec = assist.getVector<int64_t>(section, key);
                    if (vec.size() == 2)
                    {
                        uint64_t off = begin + (uint64_t)vec[0];
                        uint64_t len = (uint64_t)vec[1];
                        if (off + len > str.size())
                        {
                            return -1;
                        }
                        ini_.setKey(section, key, str.substr(off, len));
                    }
                }
            }
            return 0;
        }
        return -1;
    }

    // Parse only the index section from file; binary values are read lazily on demand.
    // Handles files of any size without loading the entire content into RAM.
    int parseFile(const std::string& filename)
    {
        if (fp_) { fclose(fp_); fp_ = nullptr; }
        file_mode_ = false;

        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp) { return -1; }

        char header[32] = {};
        if (fread(header, 1, head_size_, fp) < (size_t)head_size_) { fclose(fp); return -1; }
        if (strncmp(header, head_, 11) != 0) { fclose(fp); return -1; }

        uint64_t size_ini = 0;
        if (fread(&size_ini, sizeof(uint64_t), 1, fp) != 1) { fclose(fp); return -1; }

        std::string ini_text(size_ini, '\0');
        if (fread(ini_text.data(), 1, size_ini, fp) < size_ini) { fclose(fp); return -1; }

        file_index_.loadString(ini_text);
        binary_begin_ = (uint64_t)head_size_ + sizeof(uint64_t) + size_ini;
        file_path_ = filename;
        fp_ = fp;
        file_mode_ = true;
        return 0;
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
        return filefunc::writeStringToFile(to_string(), filename);
    }

    int load(const std::string& filename)
    {
        return parse(filefunc::readFileToString(filename));
    }

    std::string get_value(const std::string& key)
    {
        if (file_mode_)
        {
            auto vec = file_index_.getVector<int64_t>("", key);
            if (vec.size() < 2 || !fp_) { return ""; }
            uint64_t offset = binary_begin_ + (uint64_t)vec[0];
            uint64_t len = (uint64_t)vec[1];
#ifdef _WIN32
            if (_fseeki64(fp_, (int64_t)offset, SEEK_SET) != 0) { return ""; }
#else
            if (fseeko(fp_, (off_t)offset, SEEK_SET) != 0) { return ""; }
#endif
            std::string result(len, '\0');
            if (fread(result.data(), 1, len, fp_) < len) { return ""; }
            return result;
        }
        return ini_.getString("", key);
    }

    void set_value(const std::string& key, const std::string& value)
    {
        ini_.setKey("", key, value);
    }

    bool has_value(const std::string& key)
    {
        if (file_mode_)
            return file_index_.hasKey("", key);
        return ini_.hasKey("", key);
    }
};
