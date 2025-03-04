#pragma once
#include <string>
#ifdef _WIN32
#include "shlobj_core.h"
#include "strcvt.h"
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
    std::wstring path;
    HRESULT hr = path_from_shortcut(strcvt::CvtLocalStringToWString(lnk), path);
    if (SUCCEEDED(hr))
    {
        return strcvt::CvtWStringToLocalString(path);
    }
    return "";
}
#else
inline std::string target_of_lnk(const std::string &lnk)
{
    return "";
}
#endif
