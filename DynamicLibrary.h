#pragma once
#include <map>
#include <string>

class DynamicLibrary
{
private:
    static DynamicLibrary dl_;

public:
    DynamicLibrary();
    ~DynamicLibrary();

private:
    std::map<std::string, void*> dynamic_libraries_;

public:
    static void* loadDynamicLibrary(std::string library_name);
    static void* getFunction(std::string library_name, std::string function_name);
};
