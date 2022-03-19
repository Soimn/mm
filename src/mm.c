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

global Arena* Win32_Arena = 0;

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
System_FreeMemory(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

internal bool
System_OpenFile(String path, File_Handle* handle)
{
    bool succeeded = false;
    
    Arena_Marker marker = Arena_BeginTemp(Win32_Arena);
    
    int required_size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCSTR)path.data, (DWORD)path.size, 0, 0);
    
    if (required_size != 0)
    {
        LPWSTR wide_path = Arena_PushSize(Win32_Arena, required_size*sizeof(WCHAR), ALIGNOF(u8));
        
        required_size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCSTR)path.data, (DWORD)path.size, wide_path, required_size);
        
        if (required_size != 0)
        {
            HANDLE h = CreateFileW(wide_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            
            if (h != INVALID_HANDLE_VALUE)
            {
                *handle = (File_Handle)h;
                succeeded = true;
            }
        }
    }
    
    Arena_EndTemp(Win32_Arena, marker);
    
    return succeeded;
}

internal bool
System_ReadFile(File_Handle handle, Arena* arena, String* string)
{
    bool succeeded = false;
    
    DWORD high_file_size = 0;
    DWORD file_size = GetFileSize((HANDLE)handle, &high_file_size);
    
    if (high_file_size == 0 && file_size != INVALID_FILE_SIZE)
    {
        Arena_Marker marker = Arena_BeginTemp(arena);
        
        string->size = file_size;
        string->data = Arena_PushSize(arena, file_size, ALIGNOF(u8));
        
        DWORD bytes_read = 0;
        if (ReadFile((HANDLE)handle, string->data, file_size, &bytes_read, 0) != 0 && bytes_read == file_size)
        {
            Arena_ReifyTemp(arena, marker);
            succeeded = true;
        }
        else
        {
            Arena_EndTemp(arena, marker);
            ZeroStruct(string);
        }
    }
    
    return succeeded;
}

internal bool
System_FileHandlesAreEqual(File_Handle a, File_Handle b)
{
    //return (CompareObjectHandles((HANDLE)a, (HANDLE)b) == TRUE);
    NOT_IMPLEMENTED;
    return false;
}

internal void
System_CloseFile(File_Handle handle)
{
    CloseHandle((HANDLE)handle);
}

void __stdcall
WinMainCRTStartup()
{
    Win32_Arena = Arena_Init(GB(1));
    
    MM_Init();
    
    bool succeeded = MM_IncludeFile(STRING("test.m"));
    
    OutputDebugStringA(succeeded ? "\n\nsucceeded\n" : "\n\nfailed\n");
    
    ExitProcess(0);
}