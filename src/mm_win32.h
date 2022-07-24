void*
MM_System_DefaultReserveMemory(MM_umm size)
{
    void* VirtualAlloc(void* lpAddress, unsigned __int64 dwSize, unsigned __int32 flAllocationType, unsigned __int32 flProtect);
    return VirtualAlloc(0, size, 0x00002000 /*MEM_RESERVE*/, 0x04 /*PAGE_READWRITE*/);
}

void
MM_System_DefaultCommitMemory(void* ptr, MM_umm size)
{
    void* VirtualAlloc(void* lpAddress, unsigned __int64 dwSize, unsigned __int32 flAllocationType, unsigned __int32 flProtect);
    VirtualAlloc(ptr, size, 0x00001000 /*MEM_COMMIT*/, 0x04 /*PAGE_READWRITE*/);
}

void
MM_System_DefaultFreeMemory(void* ptr)
{
    int VirtualFree(void* lpAddress, unsigned __int64 dwSize, unsigned __int32 dwFreeType);
    VirtualFree(ptr, 0, 0x00008000 /*MEM_RELEASE*/);
}