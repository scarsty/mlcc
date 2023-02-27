#pragma once

#ifndef _CHECKDEPENDENCY_H_
#define _CHECKDEPENDENCY_H_
#endif

#include "windows.h"

#include "ImageHlp.h"
#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "imagehlp.lib")

// auto dlls = CheckDependency(pe_name).DllsNotGood();
// Return value is a map. If all are OK, the map is empty.
// Else, the keys are the names of dlls and the values are vectors of struct NotGoodInfo.
// Empty full path means the file is not found.
// If the path and machine of a file are both OK, but some functions are lost, usually that means the version of this file is not right.

class CheckDependency
{
private:
    PLOADED_IMAGE image = nullptr;
    struct ImportInfo
    {
        std::string full_path;
        std::string machine;
        std::vector<std::string> used_functions;    // functions used by other modules
    };
    struct NotGoodInfo
    {
        std::string full_path;
        std::string machine;
        std::vector<std::string> lost_functions;    // functions which should exist
    };
    std::map<std::string, ImportInfo> import_table_;
    std::map<std::string, NotGoodInfo> dlls_not_good_;
    std::map<std::string, std::map<std::string, int>> export_table_;

    PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva)
    {
        auto section = IMAGE_FIRST_SECTION(image->FileHeader);
        unsigned i;

        for (i = 0; i < image->FileHeader->FileHeader.NumberOfSections; i++, section++)
        {
            // This 3 line idiocy is because Watcom's linker actually sets the
            // Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
            DWORD size = section->Misc.VirtualSize;
            if (0 == size)
            {
                size = section->SizeOfRawData;
            }

            // Is the RVA within this section?
            if ((rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
            {
                return section;
            }
        }

        return 0;
    }

    LPVOID GetPtrFromRVA(DWORD rva)
    {
        INT delta;
        auto pSectionHdr = GetEnclosingSectionHeader(rva);
        if (!pSectionHdr)
        {
            return 0;
        }
        delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
        return (PVOID)(image->MappedAddress + rva - delta);
    }

    void DumpTable(const std::string& path, std::map<std::string, int>& check_map)
    {
        check_map[path]++;
        image = ImageLoad(path.c_str(), 0);
        char full_path[MAX_PATH] = {0};
        SearchPathA(NULL, path.c_str(), NULL, MAX_PATH, full_path, NULL);
        import_table_[path].full_path = full_path;
        if (!image)
        {
            return;
        }
        if (image->FileHeader->FileHeader.Machine == 0x014c)
        {
            // if the machine is not x64, the export/import table maybe not right
            import_table_[path].machine = "x86";
        }
        else if (image->FileHeader->FileHeader.Machine == 0x8664)
        {
            import_table_[path].machine = "x64";
        }
        else
        {
            import_table_[path].machine = "unknown";
        }
        if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 1)
        {
            auto export_dir = (PIMAGE_EXPORT_DIRECTORY)GetPtrFromRVA(image->FileHeader->OptionalHeader.DataDirectory[0].VirtualAddress);
            if (!export_dir)
            {
                return;
            }
            auto name = (PDWORD)GetPtrFromRVA(export_dir->AddressOfNames);
            for (size_t i = 0; i < export_dir->NumberOfNames; i++)
            {
                //printf("export %s::%s\n", path.c_str(), (const char*)GetPtrFromRVA(*name));
                if (!name)
                {
                    break;
                }
                export_table_[path][(const char*)GetPtrFromRVA(*name)]++;
                name++;
            }
        }

        if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 2)
        {
            auto import_desc = (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRVA(image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

            if (!import_desc)
            {
                return;
            }
            while (1)
            {
                // See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
                if ((import_desc->TimeDateStamp == 0) && (import_desc->Name == 0))
                {
                    break;
                }
                auto dll_name = (const char*)GetPtrFromRVA(import_desc->Name);
                auto thunk = (PIMAGE_THUNK_DATA64)GetPtrFromRVA(import_desc->OriginalFirstThunk);
                while (1)
                {
                    if (thunk->u1.AddressOfData == 0)
                    {
                        break;
                    }
                    auto name = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunk->u1.AddressOfData);
                    if (name && name->Name)
                    {
                        //printf("import %s::%s\n", dll_name, name->Name);
                        import_table_[dll_name].used_functions.push_back(name->Name);
                    }
                    thunk++;
                }
                if (check_map.count(dll_name) == 0)
                {
                    check_map[dll_name] = 0;
                }
                import_desc++;
            }
        }
        ImageUnload(image);
    }

public:
    CheckDependency()
    {
    }
    CheckDependency(const std::string& file)
    {
        dlls_not_good_ = Check(file);
    }
    std::map<std::string, NotGoodInfo> DllsNotGood()
    {
        return dlls_not_good_;
    }
    std::map<std::string, NotGoodInfo> Check(const std::string& file)
    {
        std::map<std::string, int> check_map;
        DumpTable(file.c_str(), check_map);
        while (1)
        {
            bool recheck = false;
            for (auto& pair : check_map)
            {
                if (pair.second == 0)
                {
                    DumpTable(pair.first.c_str(), check_map);
                    recheck = true;
                }
            }
            if (!recheck)
            {
                break;
            }
        }
        std::map<std::string, NotGoodInfo> problem_dlls;
        for (auto& import_pair : import_table_)
        {
            auto& dll_name = import_pair.first;
            if (dll_name.find("api-ms-win") == 0 || dll_name.find("KERNEL32") == 0)
            {
                continue;
            }
            if (import_pair.second.full_path.empty())
            {
                problem_dlls[dll_name] = NotGoodInfo();
            }
            for (auto& function_name : import_pair.second.used_functions)
            {
                if (export_table_[dll_name].count(function_name) == 0)
                {
                    problem_dlls[dll_name].lost_functions.push_back(function_name);
                }
            }
        }
        for (auto& dll : problem_dlls)
        {
            dll.second.full_path = import_table_[dll.first].full_path;
            dll.second.machine = import_table_[dll.first].machine;
        }
        return problem_dlls;
    }
};