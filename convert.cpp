#include "convert.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define vsprintf vsprintf_s
//#define fopen fopen_s
#endif

std::string convert::readStringFromFile(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, 0);
        std::string str;
        str.resize(length, '\0');
        fread((void*)str.c_str(), 1, length, fp);
        fclose(fp);
        return str;
    }
    fprintf(stderr, "Cannot open file %s!\n", filename.c_str());
    return "";
}

int convert::writeStringToFile(const std::string& str, const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        int length = str.length();
        fwrite(str.c_str(), 1, length, fp);
        fclose(fp);
        return length;
    }
    fprintf(stderr, "Cannot write file %s!\n", filename.c_str());
    return -1;
}

void convert::replaceOneSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 /*=0*/)
{
    if (oldstring.empty() || oldstring == newstring)
    {
        return;
    }
    auto pos = s.find(oldstring, pos0);
    if (pos != std::string::npos)
    {
        s.erase(pos, oldstring.length());
        s.insert(pos, newstring);
    }
}

void convert::replaceAllSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring)
{
    if (oldstring.empty() || oldstring == newstring)
    {
        return;
    }
    auto pos = s.find(oldstring);
    while (pos != std::string::npos)
    {
        if (newstring == "")
        {
            s.erase(pos, oldstring.length());
        }
        else if (oldstring.length() == newstring.length())
        {
            memcpy((void*)(s.data() + pos), newstring.data(), newstring.length());
        }
        else if (oldstring.length() < newstring.length())
        {
            memcpy((void*)(s.data() + pos), newstring.data(), oldstring.length());
            s.insert(pos + oldstring.length(), newstring.substr(oldstring.length()));
        }
        else
        {
            memcpy((void*)(s.data() + pos), newstring.data(), newstring.length());
            s.erase(pos + newstring.length(), oldstring.length() - newstring.length());
        }
        pos = s.find(oldstring, pos + newstring.length());
    }
}

std::string convert::replaceOneSubString(const std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 /*= 0*/)
{
    std::string s1 = s;
    replaceOneSubStringRef(s1, oldstring, newstring, pos0);
    return s1;
}

std::string convert::replaceAllSubString(const std::string& s, const std::string& oldstring, const std::string& newstring)
{
    std::string s1 = s;
    replaceAllSubStringRef(s1, oldstring, newstring);
    return s1;
}

void convert::replaceOneStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring)
{
    std::string s = readStringFromFile(oldfilename);
    if (s.length() <= 0)
    {
        return;
    }
    replaceOneSubStringRef(s, oldstring, newstring);
    writeStringToFile(s, newfilename);
}

void convert::replaceAllStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring)
{
    std::string s = readStringFromFile(oldfilename);
    if (s.length() <= 0)
    {
        return;
    }
    replaceAllSubStringRef(s, oldstring, newstring);
    writeStringToFile(s, newfilename);
}

std::string convert::findANumber(const std::string& s)
{
    bool findPoint = false;
    bool findNumber = false;
    bool findE = false;
    std::string n;
    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        if (c >= '0' && c <= '9' || c == '-' || c == '.' || c == 'e' || c == 'E')
        {
            if (c >= '0' && c <= '9' || c == '-')
            {
                findNumber = true;
                n += c;
            }
            if (c == '.')
            {
                if (!findPoint)
                {
                    n += c;
                }
                findPoint = true;
            }
            if (c == 'e' || c == 'E')
            {
                if (findNumber && !(findE))
                {
                    n += c;
                    findE = true;
                }
            }
        }
        else
        {
            if (findNumber)
            {
                break;
            }
        }
    }
    return n;
}

unsigned convert::findTheLast(const std::string& s, const std::string& content)
{
    size_t pos = 0, prepos = 0;
    while (pos != std::string::npos)
    {
        prepos = pos;
        pos = s.find(content, prepos + 1);
        //printf("%d\n",pos);
    }
    return prepos;
}

std::vector<std::string> convert::splitString(std::string str, std::string pattern, bool ignore_psspace)
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

bool convert::isProChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'z') || (c >= '(' && c <= ')');
}

std::string convert::toLowerCase(const std::string& s)
{
    std::string s1 = s;
    std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
    return s1;
}

std::string convert::toUpperCase(const std::string& s)
{
    std::string s1 = s;
    std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    return s1;
}

std::vector<std::string> convert::extractFormatString(const std::string& format_str)
{
    std::vector<std::string> format_strs;
    size_t p1 = 0, p2 = 0;
    while (p1 != std::string::npos)
    {
        p1 = format_str.find_first_of("%", p1);
        if (p1 != std::string::npos)
        {
            p2 = format_str.find_first_of("diuoxXfFeEgGaAcspn", p1 + 1);
            if (p2 != std::string::npos)
            {
                if (format_str.substr(p1 + 1, p2 - p1 - 1).find_first_not_of("0123456789.+-*#l") == std::string::npos)
                {
                    //find one format string
                    format_strs.push_back(format_str.substr(p1, p2 - p1 + 1));
                    p1 = p2 + 1;
                }
                else
                {
                    //not a format string, jump 2 char
                    p1 += 2;
                }
            }
            else
            {
                //not a format string, jump 2 char
                p1 += 2;
            }
        }
        if (p1 >= format_str.size())
        {
            break;
        }
    }
    return format_strs;
}
