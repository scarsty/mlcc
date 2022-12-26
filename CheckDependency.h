#pragma once

#include "windows.h"    //DONT REMOVE IT

#include "DynamicLibrary.h"
#include "ImageHlp.h"
#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "imagehlp.lib")

// auto dlls = CheckDependency(pe_name).DllsNotGood();
// return value is the dlls which lost functions

class CheckDependency
{
private:
    PLOADED_IMAGE image = nullptr;

    std::map<std::string, std::vector<std::string>> import_table_, not_good_dlls_;
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

    void DumpImportTable(const std::string& path, std::map<std::string, int>& check_map)
    {
        check_map[path]++;
        image = ImageLoad(path.c_str(), 0);
        if (!image)
        {
            return;
        }

        if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 1)
        {
            auto exportDesc = (PIMAGE_EXPORT_DIRECTORY)GetPtrFromRVA(image->FileHeader->OptionalHeader.DataDirectory[0].VirtualAddress);
            if (!exportDesc)
            {
                return;
            }
            auto name = (PDWORD)GetPtrFromRVA(exportDesc->AddressOfNames);
            for (size_t i = 0; i < exportDesc->NumberOfNames; i++)
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
            auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRVA(image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

            if (!importDesc)
            {
                return;
            }
            while (1)
            {
                // See if we've reached an empty IMAGE_IMPORT_DESCRIPTOR
                if ((importDesc->TimeDateStamp == 0) && (importDesc->Name == 0))
                {
                    break;
                }
                auto dll_name = (const char*)GetPtrFromRVA(importDesc->Name);
                auto thunk = (PIMAGE_THUNK_DATA64)GetPtrFromRVA(importDesc->OriginalFirstThunk);
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
                        import_table_[dll_name].push_back(name->Name);
                    }
                    thunk++;
                }
                if (check_map.count(dll_name) == 0)
                {
                    check_map[dll_name] = 0;
                }
                importDesc++;
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
        not_good_dlls_ = Check(file);
    }
    std::map<std::string, std::vector<std::string>> DllsNotGood()
    {
        return not_good_dlls_;
    }
    std::map<std::string, std::vector<std::string>> Check(const std::string& file)
    {
        std::map<std::string, int> check_map;
        DumpImportTable(file.c_str(), check_map);
        while (1)
        {
            bool recheck = false;
            for (auto& pair : check_map)
            {
                if (pair.second == 0)
                {
                    DumpImportTable(pair.first.c_str(), check_map);
                    recheck = true;
                }
            }
            if (!recheck)
            {
                break;
            }
        }
        std::map<std::string, std::vector<std::string>> problem_dlls;
        for (auto& import_pair : import_table_)
        {
            auto& dll_name = import_pair.first;
            if (dll_name.find("api-ms-win") == 0 || dll_name.find("KERNEL32") == 0)
            {
                continue;
            }
            for (auto& function_name : import_pair.second)
            {
                if (export_table_[dll_name].count(function_name) == 0)
                {
                    problem_dlls[dll_name].push_back(function_name);
                }
            }
        }

        return problem_dlls;
    }
};