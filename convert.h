#pragma once
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class convert
{
public:
    //string functions
    static std::string readStringFromFile(const std::string& filename);
    static int writeStringToFile(const std::string& str, const std::string& filename);
    static void writeStringAppendToFile(const std::string& str, FILE* fp);
    static void replaceOneSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 = 0);
    static void replaceAllSubStringRef(std::string& s, const std::string& oldstring, const std::string& newstring);
    static std::string replaceOneSubString(const std::string& s, const std::string& oldstring, const std::string& newstring, int pos0 = 0);
    static std::string replaceAllSubString(const std::string& s, const std::string& oldstring, const std::string& newstring);
    static void replaceOneStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring);
    static void replaceAllStringInFile(const std::string& oldfilename, const std::string& newfilename, const std::string& oldstring, const std::string& newstring);
    static std::string findANumber(const std::string& s);
    static unsigned findTheLast(const std::string& s, const std::string& content);
    static std::vector<std::string> splitString(std::string str, std::string pattern = "", bool ignore_psspace = true);
    static bool isProChar(char c);
    static std::string toLowerCase(const std::string& s);
    static std::string toUpperCase(const std::string& s);

    template <typename T>
    static int findNumbers(const std::string& s, std::vector<T>* data)
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
    static int findNumbers(const std::string& s, std::vector<T>& data)
    {
        return findNumbers(s, &data);
    }

    template <typename T>
    static std::vector<T> findNumbers(const std::string& s)
    {
        std::vector<T> data;
        findNumbers(s, &data);
        return data;
    }

private:
    static void check_format1(std::vector<std::string>& strs, int i) { }
    template <typename T, typename... Args>
    static void check_format1(std::vector<std::string>& strs, int i, T t, Args... args)
    {
        auto& s = strs[i];
        switch (s.back())
        {
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            //if (s.find("l") == std::string::npos && sizeof(T) > sizeof(int))
            //{
            //    auto error_str = std::string("Check format: too long data has been supplied!");
            //    std::cerr << error_str << std::endl;
            //    throw std::runtime_error(error_str);
            //}
            break;
        case 'c':
            break;
        case 's':
            //only check %s because it is dangerous
            if (std::is_same<const char*, T>::value || std::is_same<char*, T>::value)
            {

            }
            else
            {
                auto error_str = std::string("Check format: need %s, but ") + typeid(T).name() + " supplied!";
                std::cerr << error_str << std::endl;
                throw std::runtime_error(error_str);
            }
            break;
        default:
            break;
        }
        check_format1(strs, i + 1, args...);
    }

public:
    template <typename... Args>
    static void checkFormatStr(const std::string& format_str, Args... args)
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
        if (format_strs.size() != sizeof...(args))
        {
            auto error_str = "Check format: need " + std::to_string(format_strs.size()) + " parameter(s), but " + std::to_string(sizeof...(args)) + " supplied!";
            std::cerr << error_str << std::endl;
            throw std::runtime_error(error_str);
        }
        else
        {
            check_format1(format_strs, 0, args...);
        }
    }

    template <typename... Args>
    static std::string formatString(Args... args)
    {
#ifdef _DEBUG
        checkFormatStr(args...);
#endif
        char c[1024];
        int len = snprintf(c, sizeof(c), args...);
        std::string s(c);
        if (len >= sizeof(c))
        {
            auto c1 = new char[len + 1];
            snprintf(c1, len, args...);
            s = c1;
            delete[] c1;
        }
        return s;
    }

    template <typename T>
    static std::string vectorToString(const std::vector<T>& v, const std::string format = "", const std::string& split = ", ", const std::string end = "")
    {
        std::string s;
        std::string format1 = format;
        if (format1 == "")
        {
            if (std::is_same<const char*, T>::value || std::is_same<char*, T>::value)
            {
                format1 = "%s";
            }
            else if (std::is_same<float, T>::value || std::is_same<double, T>::value || std::is_same<long double, T>::value)
            {
                format1 = "%g";
            }
            else
            {
                format1 = "%d";
            }
        }

        for (size_t i = 0; i < v.size(); i++)
        {
            if (std::is_same<std::string, T>::value)
            {
                s += v[i];
            }
            else
            {
                s += formatString(format1.c_str(), v[i]);
            }
            if (i < v.size() - 1)
            {
                s += split;
            }
        }
        s += end;
        return s;
    }
};
