#pragma once

#include <vector>

// the turn of the next 4 lines cannot be changed
#define INITGUID
#include <objbase.h>

#include <ntddvdeo.h>

#include <windows.h>    //after ntddvdeo.h

struct GPUDeviceLUID
{
    LUID luid;
    int pcibus;
};

// Get GPU Devices
// PCIBus is the bus number of the GPU device, if it is 0, it may be a virtual device
std::vector<GPUDeviceLUID> get_gpu_devices_luid();

// Get LUID from PCIBus, only for GPU devices
LUID get_luid_from_pcibus(int pcibus);

// GPU Device Memory(VRAM)
// physical means the memory that is dedicated to the GPU, shared means the memory that is shared with the system
// physical为硬件上的显存占用，shared为系统共享显存的占用
int get_free_mem_by_luid(LUID luid, size_t* physical, size_t* shared);

// GPU Temperature
float get_temperature_by_luid(LUID luid);

// GPU Device Memory Usage by PCIBus
int get_free_mem_by_pcibus(int pcibus, size_t* resident, size_t* shared);

// CPU Host Memory(RAM)
bool get_free_host_mem(size_t* totalMemory, size_t* freeMemory, size_t* totalVirtualMemory, size_t* freeVirtualMemory);
