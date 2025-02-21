#pragma once
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace strcvt
{
// string coding/type convertor functions
#ifdef _MSC_VER
std::string CvtStringToUTF8(const std::string& localstr);
std::string CvtUTF8ToLocal(const std::string& utf8str);
std::wstring CvtUTF8ToWChar(const std::string& utf8str, int utf8strlen = -1);
std::wstring CvtLocalStringToWString(const std::string& string);
std::string CvtWStringToLocalString(const std::wstring& wstring);
#endif
std::string CvtStringToUTF8(const char16_t& src);
std::string CvtStringToUTF8(const std::u16string& src);
std::string CvtStringToUTF8(const wchar_t* start, std::uint64_t len);
std::string CvtStringToUTF8(const std::wstring& str);
std::u16string CvtStringToUTF16(const std::string& src);
std::u16string CvtStringToUTF16(const char* start, int len);
std::wstring CvtStringToWString(const std::string& src);
std::wstring CvtStringToWString(const char* start, std::uint64_t len);

}    //namespace strfunc
