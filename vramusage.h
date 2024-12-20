#pragma once

#include <vector>

using LUID_PTR = const void*;

struct GPUDeviceLUID
{
    char luid[8];
    int pcibus;
};

// Get GPU Devices and their LUID
// PCIBus is the bus number of the GPU device, if it is 0, it may be a virtual device
std::vector<GPUDeviceLUID> get_gpu_devices_luid();

// Get LUID from PCIBus, only for GPU devices
// the LUID is a 64-bit value (8 bytes) that uniquely identifies the adapter
// return 0 if success, otherwise return -1
int get_luid_from_pcibus(int pcibus, void* luid);

// GPU Device Memory(VRAM)
// physical means the memory that is dedicated to the GPU, shared means the memory that is shared with the system
// physical为硬件上的显存占用，shared为系统共享显存的占用
int get_free_mem_by_luid(LUID_PTR luid_ptr, size_t* physical, size_t* shared);

// GPU Temperature
float get_temperature_by_luid(LUID_PTR luid_ptr);

// GPU Device Memory Usage by PCIBus
int get_free_mem_by_pcibus(int pcibus, size_t* resident, size_t* shared);

// CPU Host Memory(RAM)
int get_free_host_mem(size_t* totalMemory, size_t* freeMemory, size_t* totalVirtualMemory, size_t* freeVirtualMemory);
