#include "strcvt.h"
#include <codecvt>
#include <cstdint>
#include <locale>
#ifdef _MSC_VER
#define vsprintf vsprintf_s
//#define fopen fopen_s
#include <windows.h>
// include windows.h before stringapiset.h
#include <stringapiset.h>    //for MultiByteToWideChar() and WideCharToMultiByte()

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

std::string strfunc::CvtUTF8ToLocal(const std::string& utf8str)
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

#endif    // _WIN32

std::string strfunc::CvtStringToUTF8(const char16_t& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t*>(&src);
    return convert.to_bytes(p, p + 1);
}

std::string strfunc::CvtStringToUTF8(const std::u16string& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t*>(src.data());
    return convert.to_bytes(p, p + src.size());
}

std::string strfunc::CvtStringToUTF8(const wchar_t* start, std::uint64_t len)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const std::int16_t*>(start);
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
    auto p = reinterpret_cast<const char*>(src.data());
    auto str = convert.from_bytes(p, p + src.size());
    return std::u16string(str.begin(), str.end());
}

std::u16string strfunc::CvtStringToUTF16(const char* start, int len)
{
    std::wstring_convert<std::codecvt_utf8_utf16<std::int16_t>, std::int16_t> convert;
    auto p = reinterpret_cast<const char*>(start);
    auto str = convert.from_bytes(p, p + len);
    return std::u16string(str.begin(), str.end());
}

std::wstring strfunc::CvtStringToWString(const std::string& src)
{

#ifdef _MSC_VER
    return CvtUTF8ToWChar(src, -1);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    auto p = reinterpret_cast<const char*>(src.data());
    auto str = convert.from_bytes(p, p + src.size());
    return std::wstring(str.begin(), str.end());
#endif
}

std::wstring strfunc::CvtStringToWString(const char* start, uint64_t len)
{
#ifdef _MSC_VER
    return CvtUTF8ToWChar(start, len);
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    auto p = reinterpret_cast<const char*>(start);
    auto str = convert.from_bytes(p, p + len);
    return std::wstring(str.begin(), str.end());
#endif
}
