#pragma once
#include <fstream>
#include <map>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <dlfcn.h>
#endif

class DynamicLibrary
{
private:
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
        for (auto dl : dynamic_libraries_)
        {
#ifdef _WIN32
            FreeLibrary((HINSTANCE)dl.second);
#else
            dlclose(dl.second);
#endif
        }
    }

private:
    std::map<std::string, void*> dynamic_libraries_;

public:
    static void* loadDynamicLibrary(const std::string& library_name)
    {
        auto dl = getInstance();
        if (dl->dynamic_libraries_.count(library_name) <= 0)
        {
            void* hlib = nullptr;
#ifdef _WIN32
            hlib = LoadLibraryA(library_name.c_str());
#else
            hlib = dlopen(library_name.c_str(), RTLD_LAZY);
#endif
            dl->dynamic_libraries_[library_name] = hlib;
            //if (hlib)
            //{
            //    fprintf(stdout, "Loaded dynamic library %s\n", library_name.c_str());
            //}
            //else
            //{
            //    fprintf(stdout, "Failed to load dynamic library %s\n", library_name.c_str());
            //}
        }
        return dl->dynamic_libraries_[library_name];
    }
    static void* getFunction(const std::string& library_name, const std::string& function_name)
    {
        auto dl = getInstance()->loadDynamicLibrary(library_name);
        void* func = nullptr;
        if (dl)
        {
#ifdef _WIN32
            func = GetProcAddress((HINSTANCE)dl, function_name.c_str());
#else
            func = dlsym(dl, function_name.c_str());
#endif
            //fprintf(stdout, "Loaded function %s\n", function_name.c_str());
        }
        else
        {
            //fprintf(stdout, "Failed to load function %s\n", function_name.c_str());
        }
        return func;
    }
};
