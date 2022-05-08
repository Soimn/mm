#define UNICODE
#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

// NOTE: required to remove CRT
void* memset(void* ptr, int value, unsigned __int64 size);
void* memcpy(void* rdst, const void* rsrc, unsigned __int64 count);

#pragma function(memset)
#pragma function(memcpy)

void*
memset(void* ptr, int value, unsigned __int64 size)
{
    unsigned __int8* bptr = ptr;
    unsigned __int8 val   = (unsigned __int8)value;
    
    for (unsigned __int64 i = 0; i < size; ++i)
    {
        *bptr++ = val;
    }
    
    return ptr;
}

void*
memcpy(void* rdst, const void* rsrc, unsigned __int64 count)
{
    unsigned __int8* dst = (unsigned __int8*)rdst;
    const unsigned __int8* src = (const unsigned __int8*)rsrc;
    while (count--)
    {
        *dst++ = *src++;
    }
    return dst;
}


int _fltused;

#include "mm.h"

umm
System_PageSize()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    
    return info.dwPageSize;
}

void*
System_ReserveMemory(umm size)
{
    void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    
    // TODO: handle OoM
    ASSERT(result);
    
    return result;
}

void
System_CommitMemory(void* ptr, umm size)
{
    void* result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    
    // TODO: handle OoM
    ASSERT(result);
}

void
System_FreeMemory(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

void __stdcall
WinMainCRTStartup()
{
    Workspace* workspace = Workspace_Open();
    
    // TODO:
    Interned_String s0 = InternedString_FromString(workspace, STRING(""));
    Interned_String s1 = InternedString_FromString(workspace, STRING("s"));
    Interned_String s2 = InternedString_FromString(workspace, STRING("defer"));
    
    String ss0 = InternedString_ToString(workspace, s0);
    String ss1 = InternedString_ToString(workspace, s1);
    String ss2 = InternedString_ToString(workspace, s2);
    
    Workspace_Close(workspace);
    
    ExitProcess(0);
}