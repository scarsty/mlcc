#pragma once

#define INITGUID
#include <objbase.h>

#include <cfgmgr32.h>
#include <devpkey.h>
#include <ntddvdeo.h>
#include <windows.h>

#include <cstdio>
#include <d3dkmthk.h>
#include <tchar.h>

#include <vector>

#pragma comment(lib, "cfgmgr32.lib")

inline LUID get_luid_from_pcibus(int pcibus)
{
    LUID luid{ 0, 0 };

    PWCHAR deviceInterfaceList;
    ULONG deviceInterfaceListLength = 0;
    PWCHAR deviceInterface;
    if (CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (GUID*)&GUID_DISPLAY_DEVICE_ARRIVAL, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT)) { return luid; }
    std::vector<WCHAR> deviceInterfaceListBuffer(deviceInterfaceListLength);
    deviceInterfaceList = deviceInterfaceListBuffer.data();

    if (CM_Get_Device_Interface_List((GUID*)&GUID_DISPLAY_DEVICE_ARRIVAL, NULL, deviceInterfaceList, deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT)) { return luid; }

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

        if (buffer == pcibus)
        {
            D3DKMT_OPENADAPTERFROMDEVICENAME openAdapterFromDeviceName{};
            openAdapterFromDeviceName.pDeviceName = deviceInterface;
            D3DKMTOpenAdapterFromDeviceName(&openAdapterFromDeviceName);
            D3DKMT_CLOSEADAPTER closeAdapter;
            closeAdapter.hAdapter = openAdapterFromDeviceName.hAdapter;
            D3DKMTCloseAdapter(&closeAdapter);
            luid = openAdapterFromDeviceName.AdapterLuid;
            break;
        }
    }

    return luid;
}

inline int get_free_mem_by_luid(LUID luid, size_t* resident, size_t* shared)
{
    D3DKMT_QUERYSTATISTICS queryStatistics{};
    queryStatistics.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
    queryStatistics.AdapterLuid = luid;
    if (D3DKMTQueryStatistics(&queryStatistics))
    {
        //printf("D3DKMTQueryStatistics failed with %d\n", ret);
        return 1;
    }

    ULONG64 total = 0, sharedUsage = 0, residendUsage = 0;
    for (int i = 0; i < queryStatistics.QueryResult.AdapterInformation.NbSegments; i++)
    {
        D3DKMT_QUERYSTATISTICS queryStatistics2{};
        queryStatistics2.Type = D3DKMT_QUERYSTATISTICS_SEGMENT;
        queryStatistics2.AdapterLuid = luid;
        queryStatistics2.QuerySegment.SegmentId = i;
        ULONG64 commitLimit = 0;
        ULONG aperture = 0;

        if (D3DKMTQueryStatistics(&queryStatistics2) != 0)
        {
            commitLimit = queryStatistics2.QueryResult.SegmentInformation.BytesResident;
            aperture = queryStatistics2.QueryResult.SegmentInformation.Aperture;
        }
        if (aperture)
        {
            sharedUsage += commitLimit;
        }
        else
        {
            residendUsage += commitLimit;
        }
        total += commitLimit;
    }
    //printf("%g %g\n", sharedUsage / pow(2, 20), dedicatedUsage / pow(2, 20));
    if (resident) { *resident = residendUsage; }
    if (shared) { *shared = sharedUsage; }
    return 0;
}

inline int get_free_mem_by_pcibus(int pcibus, size_t* resident, size_t* shared)
{
    auto luid = get_luid_from_pcibus(pcibus);
    return get_free_mem_by_luid(luid, resident, shared);
}
