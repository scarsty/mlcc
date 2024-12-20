#include "vramusage.h"

// the turn of the next 4 lines cannot be changed
#define INITGUID
#include <objbase.h>

#include <ntddvdeo.h>

#include <windows.h>    //after ntddvdeo.h

#include <cfgmgr32.h>
#include <d3dkmthk.h>
#include <devpkey.h>
#include <tchar.h>

#include <cstdint>
#include <cstdio>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "kernel32.lib")    //for HOST Memory(RAM)

std::vector<GPUDeviceLUID> get_gpu_devices_luid()
{
    std::vector<GPUDeviceLUID> devices;
    LUID luid{ 0, 0 };

    PWCHAR deviceInterfaceList;
    ULONG deviceInterfaceListLength = 0;
    PWCHAR deviceInterface;
    if (CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (GUID*)&GUID_DISPLAY_DEVICE_ARRIVAL, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT)) { return devices; }
    std::vector<WCHAR> deviceInterfaceListBuffer(deviceInterfaceListLength);
    deviceInterfaceList = deviceInterfaceListBuffer.data();

    if (CM_Get_Device_Interface_List((GUID*)&GUID_DISPLAY_DEVICE_ARRIVAL, NULL, deviceInterfaceList, deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT)) { return devices; }

    auto p = deviceInterfaceList;
    while (*p != L'\0')
    {
        deviceInterface = p;
        p += wcslen(p) + 1;
#ifdef _DEBUG
        //wprintf(L"%s\n", deviceInterface);
#endif

        DEVPROPTYPE devicePropertyType;
        DEVINST deviceInstanceHandle;
        ULONG deviceInstanceIdLength = MAX_DEVICE_ID_LEN;
        WCHAR deviceInstanceId[MAX_DEVICE_ID_LEN];

        if (CM_Get_Device_Interface_Property(deviceInterface, &DEVPKEY_Device_InstanceId, &devicePropertyType, (PBYTE)deviceInstanceId, &deviceInstanceIdLength, 0)) { continue; }
        if (CM_Locate_DevNode(&deviceInstanceHandle, deviceInstanceId, CM_LOCATE_DEVNODE_NORMAL)) { continue; }

        ULONG buffer;
        ULONG bufferSize = sizeof(ULONG);
        DEVPROPTYPE propertyType = DEVPROP_TYPE_EMPTY;

        if (CM_Get_DevNode_Property(deviceInstanceHandle, &DEVPKEY_Device_BusNumber, &propertyType, (PBYTE)&buffer, &bufferSize, 0)) { continue; }

        D3DKMT_OPENADAPTERFROMDEVICENAME openAdapterFromDeviceName{};
        openAdapterFromDeviceName.pDeviceName = deviceInterface;
        D3DKMTOpenAdapterFromDeviceName(&openAdapterFromDeviceName);
        D3DKMT_CLOSEADAPTER closeAdapter;
        closeAdapter.hAdapter = openAdapterFromDeviceName.hAdapter;
        D3DKMTCloseAdapter(&closeAdapter);
        luid = openAdapterFromDeviceName.AdapterLuid;
        GPUDeviceLUID device;

        memcpy(device.luid, &luid, sizeof(LUID));
        device.pcibus = buffer;
        devices.push_back(device);
    }
    return devices;
}

int get_luid_from_pcibus(int pcibus, void* luid)
{
    auto devices = get_gpu_devices_luid();
    for (auto& device : devices)
    {
        if (device.pcibus == pcibus)
        {
            memcpy(luid, device.luid, sizeof(LUID));
            return 0;
        }
    }
    return -1;
}

int get_free_mem_by_luid(LUID_PTR luid_ptr, size_t* physical, size_t* shared)
{
    auto luid = *(LUID*)luid_ptr;
    D3DKMT_QUERYSTATISTICS queryStatistics{};
    queryStatistics.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
    queryStatistics.AdapterLuid = luid;
    if (D3DKMTQueryStatistics(&queryStatistics))
    {
        //printf("D3DKMTQueryStatistics failed with %d\n", ret);
        return 1;
    }

    ULONG64 total = 0, shared_usage = 0, physical_usage = 0;
    for (int i = 0; i < queryStatistics.QueryResult.AdapterInformation.NbSegments; i++)
    {
        D3DKMT_QUERYSTATISTICS queryStatistics2{};
        queryStatistics2.Type = D3DKMT_QUERYSTATISTICS_SEGMENT;
        queryStatistics2.AdapterLuid = luid;
        queryStatistics2.QuerySegment.SegmentId = i;
        if (D3DKMTQueryStatistics(&queryStatistics2) == 0)
        {
            auto commit_limit = queryStatistics2.QueryResult.SegmentInformation.BytesResident;
            auto aperture = queryStatistics2.QueryResult.SegmentInformation.Aperture;
            if (aperture)
            {
                shared_usage += commit_limit;
            }
            else
            {
                physical_usage += commit_limit;
            }
            total += commit_limit;
        }
    }
    //printf("%g %g\n", sharedUsage / pow(2, 20), dedicatedUsage / pow(2, 20));
    if (physical) { *physical = physical_usage; }
    if (shared) { *shared = shared_usage; }
    return 0;
}

float get_temperature_by_luid(LUID_PTR luid_ptr)
{
    D3DKMT_QUERYSTATISTICS queryStatistics{};
    queryStatistics.Type = D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER;
    queryStatistics.AdapterLuid = *(LUID*)luid_ptr;
    if (D3DKMTQueryStatistics(&queryStatistics))
    {
        //printf("D3DKMTQueryStatistics failed with %d\n", ret);
        return 1;
    }
    return queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature / 10.0;
}

int get_free_mem_by_pcibus(int pcibus, size_t* resident, size_t* shared)
{
    LUID luid;
    get_luid_from_pcibus(pcibus, &luid);
    return get_free_mem_by_luid(&luid, resident, shared);
}

int get_free_host_mem(size_t* totalMemory, size_t* freeMemory, size_t* totalVirtualMemory, size_t* freeVirtualMemory)
{
    MEMORYSTATUSEX memoryInfo;
    memoryInfo.dwLength = sizeof(memoryInfo);
    if (!GlobalMemoryStatusEx(&memoryInfo))
    {
        return -1;
    }

    if (totalMemory) { *totalMemory = memoryInfo.ullTotalPhys; }
    if (freeMemory) { *freeMemory = memoryInfo.ullAvailPhys; }
    if (totalVirtualMemory) { *totalVirtualMemory = memoryInfo.ullTotalVirtual; }
    if (freeVirtualMemory) { *freeVirtualMemory = memoryInfo.ullAvailVirtual; }

    return 0;
}
