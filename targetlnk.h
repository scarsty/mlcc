#pragma once
#include <string>
#ifdef _WIN32
#include "shlobj_core.h"
#include "windows.h"
#include "wrl.h"

class CoInitializeHelper
{
public:
    CoInitializeHelper()
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    ~CoInitializeHelper()
    {
        CoUninitialize();
    }
};

inline HRESULT path_from_shortcut(const std::wstring &shortcut_filename, std::wstring &path)
{
    Microsoft::WRL::ComPtr<IShellLinkW> shell_link;
    CoInitializeHelper co_helper;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shell_link));

    if (FAILED(hr))
    {
        return hr;
    }

    Microsoft::WRL::ComPtr<IPersistFile> persistFile;    // = (IPersistFile *)shell_link;
    hr = shell_link.As(&persistFile);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = persistFile->Load(shortcut_filename.c_str(), STGM_READ);

    if (FAILED(hr))
    {
        return hr;
    }

    LPITEMIDLIST itemIdList{ NULL };
    hr = shell_link->GetIDList(&itemIdList);

    if (FAILED(hr))
    {
        return hr;
    }

    wchar_t target_path[MAX_PATH];

    hr = E_FAIL;

    if (SHGetPathFromIDList(itemIdList, target_path))
    {
        path = std::wstring(target_path);
        hr = S_OK;
    }

    if (itemIdList != NULL)
    {
        CoTaskMemFree(itemIdList);
    }
    return hr;
}

inline std::string target_of_lnk(const std::string &lnk)
{
    int wide_length = MultiByteToWideChar(CP_ACP, 0, lnk.c_str(), -1, nullptr, 0);
    if (wide_length <= 0)
    {
        return "";
    }

    std::wstring wide_lnk(static_cast<size_t>(wide_length), L'\0');
    if (MultiByteToWideChar(CP_ACP, 0, lnk.c_str(), -1, wide_lnk.data(), wide_length) <= 0)
    {
        return "";
    }
    wide_lnk.resize(static_cast<size_t>(wide_length - 1));

    std::wstring path;
    HRESULT hr = path_from_shortcut(wide_lnk, path);
    if (FAILED(hr))
    {
        return "";
    }

    int local_length = WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (local_length <= 0)
    {
        return "";
    }

    std::string local_path(static_cast<size_t>(local_length - 1), '\0');
    if (WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, local_path.data(), local_length, nullptr, nullptr) <= 0)
    {
        return "";
    }
    return local_path;
}
#else
inline std::string target_of_lnk(const std::string &lnk)
{
    return "";
}
#endif
