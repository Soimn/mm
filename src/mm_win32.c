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

global Arena* Win32_Arena;

void*
System_ReserveMemory(umm size)
{
    void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    ASSERT(result);
    
    return result;
}

void
System_CommitMemory(void* ptr, umm size)
{
    void* result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(result);
}

void
System_FreeMemory(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

void
System_Print(ZString string)
{
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), string.data, (DWORD)string.size, 0, 0);
    OutputDebugStringA((LPCSTR)string.data);
}

void __stdcall
WinMainCRTStartup()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    
    Win32_Arena = Arena_Init(System_ReserveMemory, System_CommitMemory, System_FreeMemory, sys_info.dwPageSize);
    
    Workspace_Options ws_options = {
        .ReserveMemory  = System_ReserveMemory,
        .CommitMemory   = System_CommitMemory,
        .FreeMemory     = System_FreeMemory,
        .page_size      = sys_info.dwPageSize,
    };
    Workspace* workspace = Workspace_Open(ws_options);
    
    // TODO:
    Interned_String s0 = InternedString_FromString(workspace, STRING(""));
    Interned_String s1 = InternedString_FromString(workspace, STRING("s"));
    Interned_String s2 = InternedString_FromString(workspace, STRING("defer"));
    
    String ss0 = InternedString_ToString(workspace, s0);
    //String ss1 = InternedString_ToString(workspace, s1);
    String ss2 = InternedString_ToString(workspace, s2);
    
    System_Print(ss2);
    Workspace_PrintErrors(workspace, System_Print);
    
    Workspace_Close(workspace);
    
    ExitProcess(0);
}