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

internal inline void*
System_ReserveMemory(umm size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

internal inline bool
System_CommitMemory(void* ptr, umm size)
{
    return (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
}

internal inline void
System_FreeMemory(void* ptr, umm size)
{
    VirtualFree(ptr, size, MEM_RELEASE);
}

void __stdcall
WinMainCRTStartup()
{
    MM_Init();
    
    AST_Node* ast = 0;
    ParseString(STRING("A :: 0;"), &ast, MM.ast_arena);
    
    ExitProcess(0);
}