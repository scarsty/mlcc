#pragma once
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <dlfcn.h>
#ifdef __linux__
#include <link.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

class DynamicLibrary
{
private:
#ifdef _WIN32
    static std::string getModulePath(HMODULE module)
    {
        if (!module)
        {
            return {};
        }
        std::vector<wchar_t> buffer(MAX_PATH);
        DWORD length = 0;
        while ((length = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()))) == buffer.size())
        {
            buffer.resize(buffer.size() * 2);
        }
        if (length == 0)
        {
            return {};
        }
        const int utf8_length = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), length, nullptr, 0, nullptr, nullptr);
        if (utf8_length <= 0)
        {
            return {};
        }
        std::string path(utf8_length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, buffer.data(), length, path.data(), utf8_length, nullptr, nullptr);
        return path;
    }

    static bool isPathInDirectory(const std::string& path, const std::string& directory)
    {
        if (path.size() <= directory.size())
        {
            return false;
        }
        if (_strnicmp(path.c_str(), directory.c_str(), directory.size()) != 0)
        {
            return false;
        }
        return path[directory.size()] == '\\' || path[directory.size()] == '/';
    }
#else
    static std::string normalizePath(const std::string& path)
    {
        if (path.empty())
        {
            return {};
        }
        std::error_code error;
        auto absolute_path = std::filesystem::absolute(path, error);
        return error ? path : absolute_path.lexically_normal().string();
    }

    static bool libraryNameMatches(const std::string& path, const std::string& library_name)
    {
        if (path.empty())
        {
            return false;
        }
        return path == library_name
            || std::filesystem::path(path).filename() == std::filesystem::path(library_name).filename();
    }

    static bool isPathInDirectory(const std::string& path, const std::string& directory)
    {
        if (path.size() <= directory.size() || path.compare(0, directory.size(), directory) != 0)
        {
            return false;
        }
        return path[directory.size()] == '/';
    }

#ifdef __linux__
    struct FindLoadedLibraryContext
    {
        const std::string* library_name;
        std::string path;
    };

    static int findLoadedLibrary(struct dl_phdr_info* info, size_t, void* data)
    {
        auto& context = *static_cast<FindLoadedLibraryContext*>(data);
        if (info->dlpi_name && libraryNameMatches(info->dlpi_name, *context.library_name))
        {
            context.path = normalizePath(info->dlpi_name);
            return 1;
        }
        return 0;
    }
#endif
#endif

    static DynamicLibrary* getInstance()
    {
        static DynamicLibrary dl;
        return &dl;
    }

    DynamicLibrary(DynamicLibrary&) = delete;
    DynamicLibrary& operator=(DynamicLibrary&) = delete;

private:
    DynamicLibrary()
    {
    }

    //free loaded dynamic libraries automatically when destruction
    ~DynamicLibrary()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto dl : dynamic_libraries_)
        {
            if (dl.second)
            {
#ifdef _WIN32
                FreeLibrary(dl.second);
#else
                dlclose(dl.second);
#endif
            }
        }
    }

private:
#ifdef _WIN32
    std::map<std::string, HMODULE> dynamic_libraries_;
#else
    std::map<std::string, void*> dynamic_libraries_;
#endif
    std::mutex mutex_;

    static std::string normalizeLibraryName(std::string library_name)
    {
        if (library_name.find(".") == std::string::npos)
        {
#ifdef _WIN32
            library_name += ".dll";
#elif defined(__APPLE__)
            library_name = "lib" + library_name + ".dylib";
#else
            library_name = "lib" + library_name + ".so";
#endif
        }
        if (library_name.find_first_of("/\\") != std::string::npos)
        {
            library_name = std::filesystem::absolute(library_name).string();
        }
        return library_name;
    }

    void* loadDynamicLibraryLocked(const std::string& library_name)
    {
        const auto existing = dynamic_libraries_.find(library_name);
        if (existing != dynamic_libraries_.end())
        {
            return existing->second;
        }
#ifdef _WIN32
        auto hlib = LoadLibraryA(library_name.c_str());
#else
        auto hlib = dlopen(library_name.c_str(), RTLD_LAZY);
#endif
        dynamic_libraries_[library_name] = hlib;
        return hlib;
    }

public:
    static std::string getLoadedDynamicLibraryPath(const std::string& library_name)
    {
#ifdef _WIN32
        return getModulePath(GetModuleHandleA(library_name.c_str()));
#elif defined(__linux__)
    FindLoadedLibraryContext context{ &library_name, {} };
    dl_iterate_phdr(findLoadedLibrary, &context);
    return context.path;
#elif defined(__APPLE__)
    for (uint32_t i = 0; i < _dyld_image_count(); i++)
    {
        const char* image_name = _dyld_get_image_name(i);
        if (image_name && libraryNameMatches(image_name, library_name))
        {
        return normalizePath(image_name);
        }
    }
    return {};
#else
        return {};
#endif
    }

    static bool isSystemDynamicLibraryPath(const std::string& path)
    {
#ifdef _WIN32
        std::vector<char> windows_directory(MAX_PATH);
        auto length = GetWindowsDirectoryA(windows_directory.data(), static_cast<UINT>(windows_directory.size()));
        if (length == 0 || length >= windows_directory.size())
        {
            return false;
        }
        return isPathInDirectory(path, std::string(windows_directory.data(), length));
    #elif defined(__APPLE__)
        auto normalized_path = normalizePath(path);
        return isPathInDirectory(normalized_path, "/System/Library")
            || isPathInDirectory(normalized_path, "/usr/lib");
    #elif defined(__linux__)
        auto normalized_path = normalizePath(path);
        return isPathInDirectory(normalized_path, "/lib")
            || isPathInDirectory(normalized_path, "/lib64")
            || isPathInDirectory(normalized_path, "/usr/lib")
            || isPathInDirectory(normalized_path, "/usr/lib64");
#else
        return false;
#endif
    }

    static void* loadDynamicLibrary(std::string library_name)
    {
        auto dl = getInstance();
        std::lock_guard<std::mutex> lock(dl->mutex_);
        return dl->loadDynamicLibraryLocked(normalizeLibraryName(std::move(library_name)));
    }

    static void* getFunction(const std::string& library_name, const std::string& function_name)
    {
        auto instance = getInstance();
        std::lock_guard<std::mutex> lock(instance->mutex_);
        auto library = instance->loadDynamicLibraryLocked(normalizeLibraryName(library_name));
        if (library == nullptr)
        {
            library = instance->loadDynamicLibraryLocked(normalizeLibraryName("./" + library_name));
        }
        void* func = nullptr;
        if (library)
        {
#ifdef _WIN32
            func = GetProcAddress((HINSTANCE)library, function_name.c_str());
#else
            func = dlsym(library, function_name.c_str());
#endif
            //fprintf(stdout, "Loaded function %s\n", function_name.c_str());
        }
        else
        {
            //fprintf(stdout, "Failed to load function %s\n", function_name.c_str());
        }
        return func;
    }

    static void freeDynamicLibrary(const std::string& library_name)
    {
        auto dl = getInstance();
        std::lock_guard<std::mutex> lock(dl->mutex_);
        const auto existing = dl->dynamic_libraries_.find(normalizeLibraryName(library_name));
        if (existing != dl->dynamic_libraries_.end())
        {
            auto hlib = existing->second;
#ifdef _WIN32
            FreeLibrary(hlib);
#else
            dlclose(hlib);
#endif
            dl->dynamic_libraries_.erase(existing);
        }
    }
};
