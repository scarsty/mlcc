#pragma once
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace strfunc
{
// string coding/type convertor functions
#ifdef _MSC_VER
std::string CvtStringToUTF8(const std::string& localstr);
std::string CvtUTF8ToLocal(const std::string& utf8str);
std::wstring CvtUTF8ToWChar(const std::string& utf8str, int utf8strlen = -1);
#endif
std::string CvtStringToUTF8(const char16_t& src);
std::string CvtStringToUTF8(const std::u16string& src);
std::string CvtStringToUTF8(const wchar_t* start, std::uint64_t len);
std::string CvtStringToUTF8(const std::wstring& str);
std::u16string CvtStringToUTF16(const std::string& src);
std::u16string CvtStringToUTF16(const char* start, int len);
std::wstring CvtStringToWString(const std::string& src);
std::wstring CvtStringToWString(const char* start, std::uint64_t len);

// read/write file about string functions
std::string readStringFromFile(const std::string& filename);
int writeStringToFile(const std::string& str, const std::string& filename);

void replaceOneSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 = 0);
void replaceAllSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring);
std::string replaceOneSubString(const std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 = 0);
std::string replaceAllSubString(const std::string& s, const std::string& oldstring, const std::string& newstring);
void replaceOneStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring);
void replaceAllStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring);

std::string findANumber(const std::string& s);
unsigned findTheLast(const std::string& s, const std::string& content);
std::vector<std::string> splitString(std::string str, std::string pattern = "", bool ignore_psspace = true);
bool isProChar(char c);

std::string toLowerCase(const std::string& s);
std::string toUpperCase(const std::string& s);

std::string ltrim(const std::string& s);
std::string rtrim(const std::string& s);
std::string trim(const std::string& s);

bool meet_utf8(const std::string& str);
bool meet_gbk(const std::string& str);

template <typename T>
int findNumbers(const std::string& s, std::vector<T>* data)
{
    int n = 0;
    std::string str = "";
    bool haveNum = false;
    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        bool findNumChar = (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'E' || c == 'e';
        if (findNumChar)
        {
            str += c;
            if (c >= '0' && c <= '9')
            {
                haveNum = true;
            }
        }
        if (!findNumChar || i == s.length() - 1)
        {
            if (str != "" && haveNum)
            {
                auto f = T(atof(str.c_str()));
                data->push_back(f);
                n++;
            }
            str = "";
            haveNum = false;
        }
    }
    return n;
}

template <typename T>
int findNumbers(const std::string& s, std::vector<T>& data)
{
    return findNumbers(s, &data);
}

template <typename T>
std::vector<T> findNumbers(const std::string& s)
{
    std::vector<T> data;
    findNumbers(s, &data);
    return data;
}

}    //namespace strfunc
