#define UNICODE
#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#include <shellapi.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include "mm_platform.h"

void __stdcall
WinMainCRTStartup()
{
    int num_arguments;
    LPWSTR* wide_arguments = CommandLineToArgvW(GetCommandLineW(), &num_arguments);
    
    if (num_arguments != 2)
    {
        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), "Invalid number of arguments", sizeof("Invalid number of arguments") - 1, 0, 0);
    }
    
    else
    {
        LPWSTR wide_path = wide_arguments[1];
        
        HANDLE file_handle = INVALID_HANDLE_VALUE;
        u32 file_size      = 0;
        
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        if (GetFileAttributesExW(wide_path, GetFileExInfoStandard, &attributes))
        {
            file_size = attributes.nFileSizeLow;
            file_handle = CreateFileW(wide_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, attributes.dwFileAttributes, 0);
        }
        
        void* raw_file = 0;
        if (file_handle != INVALID_HANDLE_VALUE)
        {
            void* memory = VirtualAlloc(0, file_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            DWORD bytes_read;
            if (memory != 0 && ReadFile(file_handle, memory, (u32)file_size, &bytes_read, 0) && bytes_read == file_size)
            {
                raw_file = memory;
            }
        }
        
        CloseHandle(file_handle);
        
        if (raw_file == 0)
        {
            WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), "Failed to read file", sizeof("Failed to read file") - 1, 0, 0);
        }
        
        else
        {
            umm arena_size = GB(3);
            
            DWORD garbage;
            u8* base = VirtualAlloc((void*)0, arena_size + KB(4), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (!base || !VirtualProtect(base + arena_size, KB(4), PAGE_NOACCESS, &garbage))
            {
                WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), "Failed to allocate memory", sizeof("Failed to allocate memory") - 1, 0, 0);
            }
            
            Memory_Arena arena = {
                .base_address = (u64)base,
                .size         = arena_size
            };
            
            AST_Node* statements;
            if (!ParseFile(raw_file, &arena, &statements))
            {
                WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), "Failed to parse file", sizeof("Failed to parse file") - 1, 0, 0);
            }
            
            else
            {
                Buffer out = {
                    .data = Arena_PushSize(&arena, GB(1), 1),
                    .size = GB(1)
                };
                
                u32 size = (u32)CG_GenCCodeDirectly(statements, out);
                
                WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), out.data, size, 0, 0);
            }
        }
    }
    
    ExitProcess(0);
}