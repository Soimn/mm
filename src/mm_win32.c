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

#include "mm_platform.h"

Memory_Arena Win32_Arena;
Memory_Arena Win32_Temp_Arena;

internal void
Print(char* format, ...)
{
    va_list args;
    
    Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
    
    Buffer buffer = {
        .data = Arena_PushSize(&Win32_Temp_Arena, MB(1), ALIGNOF(u8)),
        .size = MB(1),
    };
    
    va_start(args, format);
    umm req_buffer_size = String_FormatArgList(buffer, format, args);
    va_end(args);
    
    if (req_buffer_size > buffer.size)
    {
        Arena_EndTempMemory(&Win32_Temp_Arena, marker);
        marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
        
        buffer = (Buffer){
            .data = Arena_PushSize(&Win32_Temp_Arena, req_buffer_size, ALIGNOF(u8)),
            .size = req_buffer_size,
        };
        
        va_start(args, format);
        req_buffer_size = String_FormatArgList(buffer, format, args);
        va_end(args);
        
        ASSERT(req_buffer_size == buffer.size);
    }
    
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer.data, (DWORD)req_buffer_size - 1, 0, 0);
    
    Arena_EndTempMemory(&Win32_Temp_Arena, marker);
}

internal void*
System_ReserveMemory(umm size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

internal bool
System_CommitMemory(void* base_pointer, umm size)
{
    return !!VirtualAlloc(base_pointer, size, MEM_COMMIT, PAGE_READWRITE);
}

internal void
System_Exit()
{
    ExitProcess(0);
}

internal bool
System_ReadFile(Memory_Arena* arena, Path path, String* contents)
{
    bool encountered_errors = false;
    
    HANDLE file = CreateFileA((LPCSTR)path.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file == INVALID_HANDLE_VALUE) encountered_errors = true;
    else
    {
        DWORD high_size;
        DWORD size = GetFileSize(file, &high_size);
        
        if (high_size != 0) encountered_errors = true;
        else
        {
            contents->size = size;
            contents->data = Arena_PushSize(arena, contents->size, ALIGNOF(u8));
            
            DWORD bytes_read;
            if (!ReadFile(file, contents->data, (DWORD)contents->size, &bytes_read, 0) || bytes_read != contents->size)
            {
                encountered_errors = true;
            }
        }
        
        CloseHandle(file);
    }
    
    return !encountered_errors;
}

internal bool
System_ResolvePath(struct Memory_Arena* arena, Path wd, Path path, Path* resolved_dir, Path* resolved_path)
{
    bool encountered_errors = false;
    
    if (!SetCurrentDirectory((LPCSTR)wd.data)) encountered_errors = true;
    else
    {
        u32 required_size = GetFullPathNameA((LPCSTR)path.data, 0, 0, 0);
        
        Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
        
        u8* memory = Arena_PushSize(&Win32_Temp_Arena, required_size, ALIGNOF(u8));
        
        u8* filename;
        if (!GetFullPathNameA((LPCSTR)path.data, required_size, (LPSTR)memory, (LPSTR*)&filename)) encountered_errors = true;
        else
        {
            String dir = {
                .data = memory,
                .size = filename - (u8*)memory
            };
            
            String full_path = String_FromCString(memory);
            
            HANDLE handle = CreateFile((LPCSTR)memory, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            CloseHandle(handle);
            
            if (handle == INVALID_HANDLE_VALUE) encountered_errors = true;
            else
            {
                if (resolved_dir)
                {
                    *resolved_dir = (Path){
                        .data = Arena_PushSize(arena, dir.size + 1, ALIGNOF(u8)),
                        .size = dir.size
                    };
                    
                    Copy(dir.data, resolved_dir->data, dir.size);
                    resolved_dir->data[resolved_dir->size] = 0;
                }
                
                
                if (resolved_path)
                {
                    *resolved_path = (Path){
                        .data = Arena_PushSize(arena, full_path.size + 1, ALIGNOF(u8)),
                        .size = full_path.size
                    };
                    
                    Copy(full_path.data, resolved_path->data, full_path.size);
                    resolved_path->data[resolved_path->size] = 0;
                }
            }
        }
        
        Arena_EndTempMemory(&Win32_Temp_Arena, marker);
    }
    
    return !encountered_errors;
}

internal bool
System_PathsRefSameFile(Path p0, Path p1)
{
    bool result = false;
    
    HANDLE h0 = CreateFileA((LPCSTR)p0.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE h1 = CreateFileA((LPCSTR)p1.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    FILE_ID_INFO i0;
    FILE_ID_INFO i1;
    
    if (h0 == INVALID_HANDLE_VALUE                                     ||
        h1 == INVALID_HANDLE_VALUE                                     ||
        !GetFileInformationByHandleEx(h0, FileIdInfo, &i0, sizeof(i0)) ||
        !GetFileInformationByHandleEx(h1, FileIdInfo, &i1, sizeof(i1)))
    {
        // TODO: What should be done in this case?
    }
    
    else
    {
        result = StructCompare(&i0, &i1);
    }
    
    CloseHandle(h0);
    CloseHandle(h1);
    
    return result;
}

internal void
ArgError(char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
    
    umm buffer_size = String_FormatArgList((Buffer){0}, format, args);
    
    Buffer buffer = {
        .data = Arena_PushSize(&Win32_Temp_Arena, buffer_size, ALIGNOF(u8)),
        .size = buffer_size,
    };
    
    String_FormatArgList(buffer, format, args);
    
#define ERR_PREFIX "Invalid Argument Error: "
#define ERR_SUFFIX "\n"
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), ERR_PREFIX, sizeof(ERR_PREFIX) - 1, 0, 0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer.data, (u32)buffer_size, 0, 0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), ERR_SUFFIX, sizeof(ERR_SUFFIX) - 1, 0, 0);
#undef ERR_PREFIX
#undef ERR_SUFFIX
    
    Arena_EndTempMemory(&Win32_Temp_Arena, marker);
    
    va_end(args);
}

typedef struct Command_Line_Arguments
{
    Path_Label* path_labels;
    u32 path_label_count;
    String main_file;
} Command_Line_Arguments;

internal bool
ParseCommandLineArguments(u8* command_line, Command_Line_Arguments* args)
{
    bool encountered_errors = false;
    
    // NOTE: usage mm [main file] [options]
    // valid options: ...
    
    u8* scan = command_line;
    
    // TODO: Is it safe to assume this produces a valid path to core?
    Path working_dir       = {0};
    Path default_core_path = {0};
    { /// Extract path to core folder
        for (; *scan && *scan != ' '; ++scan);
        if (*scan != 0) *scan++ = 0;
        
        
        //PathRemoveFileSpecA((LPSTR)command_line);
        {
            u8* last_slash = 0;
            for (u8* path_scan = command_line; *path_scan != 0; ++path_scan)
            {
                if (*path_scan == '/' || *path_scan == '\\')
                {
                    last_slash = path_scan;
                }
            }
            
            if (last_slash != 0) *last_slash = 0;
        }
        
        String exe_path = String_FromCString(command_line);
        String addend   = STRING("\\core\\");
        
        Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
        
        String core_path;
        core_path.size = exe_path.size + addend.size;
        core_path.data = Arena_PushSize(&Win32_Temp_Arena, default_core_path.size + 1, ALIGNOF(u8));
        
        Copy(exe_path.data, core_path.data, exe_path.size);
        Copy(addend.data, core_path.data + exe_path.size, addend.size);
        core_path.data[core_path.size] = 0;
        
        u32 required_size = GetFullPathNameA((LPCSTR)core_path.data, 0, 0, 0);
        
        u8* memory = Arena_PushSize(&Win32_Arena, required_size, ALIGNOF(u8));
        
        if (!GetFullPathNameA((LPCSTR)core_path.data, required_size, (LPSTR)memory, 0)) encountered_errors = true;
        else
        {
            default_core_path = String_FromCString(memory);
            
            // TODO: WTF Windows
            for (umm i = 0; i < default_core_path.size; ++i)
            {
                if (default_core_path.data[i] == 0x22)
                {
                    default_core_path.data += i + 1;
                    default_core_path.size -= i + 1;
                }
            }
        }
        
        Arena_EndTempMemory(&Win32_Temp_Arena, marker);
        
        if (!encountered_errors)
        {
            required_size = GetCurrentDirectory(0, 0);
            
            working_dir.size = required_size,
            working_dir.data = Arena_PushSize(&Win32_Arena, required_size + 1, ALIGNOF(u8));
            
            if (!GetCurrentDirectory(required_size, (LPSTR)working_dir.data)) encountered_errors = true;
            else
            {
                working_dir.data[working_dir.size - 1] = '\\';
                working_dir.data[working_dir.size]     = 0;
            }
        }
    }
    
    if (!encountered_errors)
    {
        while (*scan != 0 && *scan == ' ') ++scan;
        
        u8* start = scan;
        
        while (*scan != 0 && *scan != ' ') ++scan;
        if (*scan != 0) *scan++ = 0;
        
        args->main_file = String_FromCString(start);
        
        if (args->main_file.size == 0)
        {
            ArgError("missing main file path");
            encountered_errors = true;
        }
    }
    
    if (!encountered_errors)
    {
        // TODO: parse options
    }
    
    args->path_labels      = Arena_PushSize(&Win32_Arena, sizeof(Path_Label) * 2, ALIGNOF(Path_Label));
    args->path_label_count = 2;
    
    args->path_labels[0] = (Path_Label){
        .label = {0},
        .path  = working_dir
    };
    
    args->path_labels[1] = (Path_Label){
        .label = STRING("core"),
        .path  = default_core_path
    };
    
    return !encountered_errors;
}

void __stdcall
WinMainCRTStartup()
{
    Win32_Arena      = Arena_Init(GB(10));
    Win32_Temp_Arena = Arena_Init(GB(10));
    
    Command_Line_Arguments args = {0};
    if (ParseCommandLineArguments((u8*)GetCommandLineA(), &args))
    {
        if (MM_Init(args.main_file, args.path_labels, args.path_label_count))
        {
            AST_Node* decl;
            while (MM_ResolveNextDecl(&decl))
            {
                if (decl->kind == AST_IncludeDecl)  Print("Include \"%S\"\n", StringLiteral_Of(decl->include.path));
                else
                {
                    AST_Node* names;
                    
                    if (decl->kind == AST_VariableDecl)
                    {
                        Print("Var ");
                        names = decl->var_decl.names;
                    }
                    
                    else
                    {
                        ASSERT(decl->kind == AST_ConstantDecl);
                        
                        Print("Const ");
                        names = decl->const_decl.names;
                    }
                    
                    for (AST_Node* name = names; name != 0; name = name->next)
                    {
                        ASSERT(name->kind == AST_Identifier);
                        Print("%S", Identifier_Of(name->identifier));
                        
                        if (name->next != 0) Print(", ");
                        else                 Print("\n");
                    }
                }
            }
            
            if (MM.encountered_errors) Print("Encountered errors\n");
        }
    }
    
    ExitProcess(0);
}