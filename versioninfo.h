#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <type_traits>

#pragma comment(lib, "version.lib")

enum InfoType
{
    InfoType_Version,
    InfoType_FileDescription,
};

inline std::wstring get_loaded_module_path(const void* address)
{
    HMODULE module = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<LPCWSTR>(address), &module))
    {
        return {};
    }

    std::wstring path(512, L'\\0');
    for (;;)
    {
        DWORD length = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0)
        {
            return {};
        }
        if (length < path.size() - 1)
        {
            path.resize(length);
            return path;
        }
        path.resize(path.size() * 2);
    }
}

namespace
{
inline void versioninfo_module_anchor() {}
}

inline HMODULE get_current_module_handle()
{
    HMODULE module = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&versioninfo_module_anchor), &module))
    {
        return nullptr;
    }
    return module;
}

inline std::wstring get_loaded_module_path()
{
    HMODULE module = get_current_module_handle();
    std::wstring path(512, L'\0');
    for (;;)
    {
        DWORD length = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0)
        {
            return {};
        }
        if (length < path.size() - 1)
        {
            path.resize(length);
            return path;
        }
        path.resize(path.size() * 2);
    }
}

inline std::string get_version(const std::string& filename);
inline std::string get_string_info(const std::string& filename, const std::string& info);

inline std::string get_loaded_module_version(const void* address)
{
    auto path = get_loaded_module_path(address);
    if (path.empty())
    {
        return {};
    }

    DWORD ignored = 0;
    DWORD size = GetFileVersionInfoSizeW(path.c_str(), &ignored);
    if (size == 0)
    {
        return {};
    }

    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoW(path.c_str(), 0, size, data.data()))
    {
        return {};
    }

    VS_FIXEDFILEINFO* info = nullptr;
    UINT info_size = 0;
    if (!VerQueryValueW(data.data(), L"\\", reinterpret_cast<LPVOID*>(&info), &info_size) || info_size == 0
        || info->dwSignature != 0xfeef04bd)
    {
        return {};
    }

    return std::to_string((info->dwFileVersionMS >> 16) & 0xffff)
        + "." + std::to_string(info->dwFileVersionMS & 0xffff)
        + "." + std::to_string((info->dwFileVersionLS >> 16) & 0xffff)
        + "." + std::to_string(info->dwFileVersionLS & 0xffff);
}

inline std::string get_loaded_module_version()
{
    auto path = get_loaded_module_path();
    if (path.empty())
    {
        return {};
    }
    return get_version(std::string(path.begin(), path.end()));
}

inline std::string get_loaded_module_file_description()
{
    auto path = get_loaded_module_path();
    if (path.empty())
    {
        return {};
    }
    return get_string_info(std::string(path.begin(), path.end()), "FileDescription");
}

inline std::string get_version(const std::string& filename)
{
    DWORD verHandle = 0;
    UINT size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize = GetFileVersionInfoSizeA(filename.c_str(), &verHandle);

    if (verSize != NULL)
    {
        std::string verData;
        verData.resize(verSize);
        if (GetFileVersionInfoA(filename.c_str(), verHandle, verSize, (LPVOID)verData.data()))
        {
            if (VerQueryValueA(verData.data(), "\\", (VOID FAR * FAR*)&lpBuffer, &size))
            {
                if (size)
                {
                    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                    {
                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS

                        return std::to_string((verInfo->dwFileVersionMS >> 16) & 0xffff)
                            + "." + std::to_string((verInfo->dwFileVersionMS >> 0) & 0xffff)
                            + "." + std::to_string((verInfo->dwFileVersionLS >> 16) & 0xffff)
                            + "." + std::to_string((verInfo->dwFileVersionLS >> 0) & 0xffff);
                    }
                }
            }
        }
    }
    return "";
}

inline std::string get_string_info(const std::string& filename, const std::string& info)
{
    DWORD verHandle = 0;
    UINT size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize = GetFileVersionInfoSizeA(filename.c_str(), &verHandle);

    if (verSize != NULL)
    {
        std::string verData;
        verData.resize(verSize);
        if (GetFileVersionInfoA(filename.c_str(), verHandle, verSize, (LPVOID)verData.data()))
        {
            if (VerQueryValue(verData.data(), TEXT("\\VarFileInfo\\Translation"), (VOID FAR * FAR*)&lpBuffer, &size))
            {
                if (size)
                {
                    // Read the list of languages and code pages.
                    struct LANGANDCODEPAGE
                    {
                        WORD wLanguage;
                        WORD wCodePage;
                    }* lpTranslate;

                    lpTranslate = (struct LANGANDCODEPAGE*)lpBuffer;
                    // Read the file description for each language and code page.
                    for (UINT i = 0; i < (size / sizeof(struct LANGANDCODEPAGE)); i++)
                    {
                        char SubBlock[50];
                        snprintf(SubBlock, 50, ("\\StringFileInfo\\%04x%04x\\" + info).c_str(), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
                        if (VerQueryValueA(verData.data(), SubBlock, (VOID FAR * FAR*)&lpBuffer, &size))
                        {
                            return (const char*)lpBuffer;
                        }
                    }
                }
            }
        }
    }
    return "";
}

inline std::string get_version(const std::string& filename, InfoType it)
{
    if (it == InfoType_Version)
    {
        return get_version(filename);
    }
    else if (it == InfoType_FileDescription)
    {
        return get_string_info(filename, "FileDescription");
    }
    return "";
}
