#pragma once

#ifdef _WIN32

#include <windows.h>

#include <tlhelp32.h>

#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "Kernel32.lib")
#endif

inline std::string loaded_dll_path_to_string(const wchar_t* path, bool utf8)
{
    const UINT code_page = utf8 ? CP_UTF8 : CP_ACP;
    const int size = WideCharToMultiByte(code_page, 0, path, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        return "";
    }

    std::string text(static_cast<size_t>(size), '\0');
    if (WideCharToMultiByte(code_page, 0, path, -1, text.data(), size, nullptr, nullptr) <= 0)
    {
        return "";
    }
    text.pop_back();
    return text;
}

// Returns absolute paths in the local code page by default, or UTF-8 if requested.
inline std::vector<std::string> loaded_dll_paths(bool utf8 = false)
{
    std::vector<std::string> paths;
    const DWORD process_id = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return paths;
    }

    MODULEENTRY32W module = {};
    module.dwSize = sizeof(module);
    if (Module32FirstW(snapshot, &module))
    {
        do
        {
            if (module.th32ProcessID == process_id && module.modBaseAddr != nullptr
                && module.modBaseAddr != reinterpret_cast<BYTE*>(GetModuleHandleW(nullptr)))
            {
                std::string path = loaded_dll_path_to_string(module.szExePath, utf8);
                if (!path.empty())
                {
                    paths.emplace_back(std::move(path));
                }
            }
        } while (Module32NextW(snapshot, &module));
    }

    CloseHandle(snapshot);
    return paths;
}

#else

#include <string>
#include <vector>

inline std::vector<std::string> loaded_dll_paths(bool = false)
{
    return {};
}

#endif
