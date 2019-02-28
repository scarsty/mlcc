#pragma once
#include <map>
#include <string>

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
    DynamicLibrary();
    ~DynamicLibrary();

private:
    std::map<std::string, void*> dynamic_libraries_;

public:
    static void* loadDynamicLibrary(std::string library_name);
    static void* getFunction(const std::string& library_name, const std::string& function_name);
};
