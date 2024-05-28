#include "strfunc.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <locale>
#include <codecvt>

#ifdef _MSC_VER
#define vsprintf vsprintf_s
//#define fopen fopen_s
#include "stringapiset.h" //for MultiByteToWideChar() and WideCharToMultiByte()
#endif


#ifdef _MSC_VER
std::string strfunc::CvtStringToUTF8(const std::string& localstr)
{
    int wlen = MultiByteToWideChar(CP_ACP, 0, localstr.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(CP_ACP, 0, localstr.c_str(), -1, wstr.data(), wlen);
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8str(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, utf8str.data(), utf8len, nullptr, nullptr);
    std::string result(utf8str.data());
    return result;
}

std::string strfunc::CvtUTF8ToLocal(const std::string &utf8str)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, wstr.data(), wlen);
    int locallen = WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> localstr(locallen);
    WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, localstr.data(), locallen, nullptr, nullptr);
    std::string result(localstr.data());
    return result;
}

std::wstring strfunc::CvtUTF8ToWChar(const std::string& utf8str, int utf8strlen)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), utf8strlen, nullptr, 0);
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), utf8strlen, wstr.data(), wlen);
    std::wstring ret(wstr.data());
    return ret;
}

#endif // _WIN32

std::string strfunc::CvtStringToUTF8(const char16_t& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t *>(&src);
    return convert.to_bytes(p, p + 1);
}


std::string strfunc::CvtStringToUTF8(const std::u16string& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t *>(src.data());
    return convert.to_bytes(p, p + src.size());
}

std::string strfunc::CvtStringToUTF8(const wchar_t* start, std::uint64_t len)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t *>(start);
    return convert.to_bytes(p, p + len);
}

std::string strfunc::CvtStringToUTF8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(str);
}

std::u16string strfunc::CvtStringToUTF16(const std::string& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const char *>(src.data());
    auto str = convert.from_bytes(p, p + src.size());
    return std::u16string(str.begin(),str.end());
}

std::u16string strfunc::CvtStringToUTF16(const char* start, int len)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const char *>(start);
    auto str = convert.from_bytes(p, p + len);
    return std::u16string(str.begin(),str.end());
}

std::wstring strfunc::CvtStringToWString(const std::string& src)
{

#ifdef _MSC_VER
    return CvtUTF8ToWChar(src, -1);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    auto p = reinterpret_cast<const char *>(src.data());
    auto str = convert.from_bytes(p, p + src.size());
    return std::wstring(str.begin(),str.end());
#endif
}

std::wstring strfunc::CvtStringToWString(const char* start, uint64_t len)
{
#ifdef _MSC_VER
    return CvtUTF8ToWChar(start, len);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    auto p = reinterpret_cast<const char *>(start);
    auto str = convert.from_bytes(p, p + len);
    return std::wstring(str.begin(), str.end());
#endif
}

std::string strfunc::readStringFromFile(const std::string& filename)
{
    FILE* fp = nullptr;
#ifdef _MSC_VER
    int err = _wfopen_s(&fp, CvtStringToWString(filename.c_str()).c_str(), CvtStringToWString("rb").c_str());
#else
    fp = fopen(filename.c_str(), "rb");
#endif
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
    FILE* fp = nullptr;
#ifdef _MSC_VER
    int err = _wfopen_s(&fp, CvtStringToWString(filename.c_str()).c_str(), CvtStringToWString("rb").c_str());
#else
    fp = fopen(filename.c_str(), "rb");
#endif
    if (fp)
    {
        int length = str.length();
        fwrite(str.c_str(), 1, length, fp);
        fflush(fp);
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
