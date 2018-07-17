#include "DynamicLibrary.h"
#include "File.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

DynamicLibrary::DynamicLibrary()
{
}

//free loaded dynamic libraries automatically when destruction
DynamicLibrary::~DynamicLibrary()
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

void* DynamicLibrary::loadDynamicLibrary(std::string library_name)
{
    auto dl = getInstance();
    std::string dl_ext;
#ifdef _WIN32
    dl_ext = "dll";
#else
    dl_ext = "so";
    if (File::getFilenameWithoutPath(library_name).find("lib") != 0)
    {
        library_name = "lib" + library_name;
    }
#endif
    if (File::getFileExt(library_name) != dl_ext)
    {
        library_name += "." + dl_ext;
    }
    if (!File::fileExist(library_name))
    {
        return nullptr;
    }
    if (dl->dynamic_libraries_.count(library_name) <= 0)
    {
        void* hlib = nullptr;
#ifdef _WIN32
        hlib = LoadLibraryA(library_name.c_str());
#else
        hlib = dlopen(library_name.c_str(), RTLD_LAZY);
#endif
        dl->dynamic_libraries_[library_name] = hlib;
        if (hlib)
        {
            fprintf(stdout, "Load dynamic library: %s\n", library_name.c_str());
        }
    }
    return dl->dynamic_libraries_[library_name];
}

void* DynamicLibrary::getFunction(std::string library_name, std::string function_name)
{
    auto h_dl = getInstance()->loadDynamicLibrary(library_name);
    void* func = nullptr;
    if (h_dl)
    {
#ifdef _WIN32
        func = GetProcAddress((HINSTANCE)h_dl, function_name.c_str());
#else
        func = dlsym(h_dl, function_name.c_str());
#endif
    }
    return func;
}
