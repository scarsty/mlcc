#include "strfunc.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define vsprintf vsprintf_s
//#define fopen fopen_s
#endif

std::string strfunc::readStringFromFile(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, 0);
        std::string str;
        str.resize(length, '\0');
        if (fread((void*)str.c_str(), 1, length, fp) < length)
        {
            //fprintf(stderr, "Read file %s unfinished!\n", filename.c_str());
        }
        fclose(fp);
        return str;
    }
    //fprintf(stderr, "Cannot open file %s!\n", filename.c_str());
    return "";
}

int strfunc::writeStringToFile(const std::string& str, const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp)
    {
        int length = str.length();
        fwrite(str.c_str(), 1, length, fp);
        fclose(fp);
        return length;
    }
    //fprintf(stderr, "Cannot write file %s!\n", filename.c_str());
    return -1;
}

void strfunc::replaceOneSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 /*=0*/)
{
    if (oldstring.empty() || oldstring == newstring)
    {
        return;
    }
    auto pos = s.find(oldstring, pos0);
    if (pos != std::string::npos)
    {
        s.replace(pos, oldstring.length(), newstring);
    }
}

void strfunc::replaceAllSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring)
{
    if (oldstring.empty() || oldstring == newstring)
    {
        return;
    }
    auto pos = s.find(oldstring);
    while (pos != std::string::npos)
    {
        s.replace(pos, oldstring.length(), newstring);
        pos = s.find(oldstring, pos + newstring.length());
    }
}

std::string strfunc::replaceOneSubString(const std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 /*= 0*/)
{
    std::string s1 = s;
    replaceOneSubStringRef(s1, oldstring, newstring, pos0);
    return s1;
}

std::string strfunc::replaceAllSubString(const std::string& s, const std::string& oldstring, const std::string& newstring)
{
    std::string s1 = s;
    replaceAllSubStringRef(s1, oldstring, newstring);
    return s1;
}

void strfunc::replaceOneStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring)
{
    std::string s = readStringFromFile(oldfilename);
    if (s.length() <= 0)
    {
        return;
    }
    replaceOneSubStringRef(s, oldstring, newstring);
    writeStringToFile(s, newfilename);
}

void strfunc::replaceAllStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring)
{
    std::string s = readStringFromFile(oldfilename);
    if (s.length() <= 0)
    {
        return;
    }
    replaceAllSubStringRef(s, oldstring, newstring);
    writeStringToFile(s, newfilename);
}

std::string strfunc::findANumber(const std::string& s)
{
    bool findPoint = false;
    bool findNumber = false;
    bool findE = false;
    std::string n;
    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == 'e' || c == 'E')
        {
            if ((c >= '0' && c <= '9') || c == '-')
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

unsigned strfunc::findTheLast(const std::string& s, const std::string& content)
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

std::vector<std::string> strfunc::splitString(std::string str, std::string pattern, bool ignore_psspace)
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

bool strfunc::isProChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'z') || (c >= '(' && c <= ')');
}

std::string strfunc::toLowerCase(const std::string& s)
{
    std::string s1 = s;
    std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
    return s1;
}

std::string strfunc::toUpperCase(const std::string& s)
{
    std::string s1 = s;
    std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    return s1;
}

std::string strfunc::ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \n\r\t\f\v");
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string strfunc::rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(" \n\r\t\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string strfunc::trim(const std::string& s)
{
    return rtrim(ltrim(s));
}

bool strfunc::meet_utf8(const std::string& str)
{
    unsigned int n = 0;
    bool all_ascii = true;
    for (const unsigned char c : str)
    {
        if (all_ascii && c < 0x20 || c >= 0x80)
        {
            all_ascii = false;
        }
        if (n == 0)
        {
            //the first of multi byte
            if (c >= 0x80)
            {
                if (c >= 0xFC && c <= 0xFD)
                {
                    n = 6;
                }
                else if (c >= 0xF8)
                {
                    n = 5;
                }
                else if (c >= 0xF0)
                {
                    n = 4;
                }
                else if (c >= 0xE0)
                {
                    n = 3;
                }
                else if (c >= 0xC0)
                {
                    n = 2;
                }
                else
                {
                    return false;
                }
                n--;
            }
        }
        else
        {
            //it should be 10xxxxxx
            if ((c & 0xC0) != 0x80)
            {
                return false;
            }
            n--;
        }
    }
    if (n != 0)
    {
        return false;
    }
    if (all_ascii)
    {
        return true;
    }
    return true;
}

bool strfunc::meet_gbk(const std::string& str)
{
    unsigned int n = 0;
    bool all_ascii = true;
    for (const unsigned char c : str)
    {
        if (all_ascii && c < 0x20 || c >= 0x80)
        {
            all_ascii = false;
        }
        if (n == 0)
        {
            if (c >= 0x80)
            {
                if (c >= 0x81 && c <= 0xFE)
                {
                    n = +2;
                }
                else
                {
                    return false;
                }
                n--;
            }
        }
        else
        {
            if (c < 0x40 || c > 0xFE)
            {
                return false;
            }
            n--;
        }
    }
    if (n != 0)
    {
        return false;
    }
    if (all_ascii)
    {
        return true;
    }
    return true;
}
