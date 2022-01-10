#include <stdarg.h>

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifndef _WIN64
#error 32 bit mode is not supported yet
#endif

typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef u64 umm;
typedef i64 imm;

// NOTE: required to remove CRT
void* memset(void* ptr, int value, umm size);
void* memcpy(void* rdst, const void* rsrc, u64 count);

#pragma function(memset)
#pragma function(memcpy)

void*
memset(void* ptr, int value, umm size)
{
    u8* bptr = ptr;
    u8 val   = (u8)value;
    
    for (umm i = 0; i < size; ++i)
    {
        *bptr++ = val;
    }
    
    return ptr;
}

void*
memcpy(void* rdst, const void* rsrc, u64 count)
{
    u8* dst = (u8*)rdst;
    const u8* src = (const u8*)rsrc;
    while (count--)
    {
        *dst++ = *src++;
    }
    return dst;
}


int _fltused;

int __stdcall
_DllMainCRTStartup(void* instance, u32 reason, void* reserved)
{
    return 1;
}

typedef u64 umm;
typedef i64 imm;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define I8_MIN  0xFF
#define I16_MIN 0xFFFF
#define I32_MIN 0xFFFFFFFF
#define I64_MIN 0xFFFFFFFFFFFFFFFF

#define I8_MAX  0x7F
#define I16_MAX 0x7FFF
#define I32_MAX 0x7FFFFFFF
#define I64_MAX 0x7FFFFFFFFFFFFFFF

typedef b8 bool;

#define false 0
#define true 1

typedef struct Buffer
{
    u8* data;
    u64 size;
} Buffer;

typedef Buffer String;
typedef String Path;

#define STRING(str) (String){ .data = (u8*)(str), .size = sizeof(str) - 1 }

#define Enum8(name)  u8
#define Enum16(name) u16
#define Enum32(name) u32
#define Enum64(name) u64

#define Flag8(name)  b8
#define Flag16(name) b16
#define Flag32(name) b32
#define Flag64(name) b64

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)

#if MM_DEBUG
#define ASSERT(EX) ((EX) ? 1 : *(volatile int*)0)
#else
#define ASSERT(EX)
#endif


#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")
#define INVALID_DEFAULT_CASE default: ASSERT(!"INVALID_DEFAULT_CASE"); break
#define INVALID_CODE_PATH ASSERT(!"INVALID_CODE_PATH")

#define OFFSETOF(element, type) (umm)&((type*)0)->element
#define ALIGNOF(T) OFFSETOF(t, struct { u8 b; T t; })

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(n) ((n) < 0 ? -(n) : (n))
#define SGN(n) ((n) < 0 ? -1 : ((n) == 0 ? 0 : 1))

#define KB(N) ((umm)(N) << 10)
#define MB(N) ((umm)(N) << 20)
#define GB(N) ((umm)(N) << 30)
#define TB(N) ((umm)(N) << 40)

#define MS(N) ((f32)N / 1000)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define XOR_SWAP(A, B) ((A) ^= ((B) ^= ((A) ^= (B))))

#define internal static
#define global static

// NOTE: This is just a hack to work around a parsing bug in 4coder
#define TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

#define INTERNED_EMPTY_STRING 0
#define BLANK_IDENTIFIER INTERNED_EMPTY_STRING
#define EMPTY_STRING INTERNED_EMPTY_STRING

typedef u32 Interned_String;

typedef u32 Character;

typedef union UTF8_Word
{
    u8 bytes[4];
    u32 word;
} UTF8_Word;

enum KEYWORD_KIND
{
    Keyword_Invalid = 0,
    
    Keyword_Do,
    Keyword_In,
    Keyword_Where,
    Keyword_Proc,
    Keyword_Struct,
    Keyword_Union,
    Keyword_Enum,
    Keyword_True,
    Keyword_False,
    Keyword_As,
    Keyword_If,
    Keyword_Else,
    Keyword_When,
    Keyword_While,
    Keyword_For,
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    Keyword_Include,
    
    KEYWORD_COUNT,
};

typedef u32 File_ID;

typedef struct File
{
    struct File* next;
    Path path;
    Path dir;
    String contents;
    File_ID id;
    
    struct
    {
        File_ID id;
        u32 offset;
    } includer;
} File;

typedef u32 Type_ID;

// NOTE: ugly hack due to 4coder parsing bug
struct Symbol;
typedef struct Symbol* _Symbol_Table;
typedef _Symbol_Table Symbol_Table;

struct Memory_Arena;
internal void System_InitArena(struct Memory_Arena* arena);
internal void System_GrowArena(struct Memory_Arena* arena, umm overflow);
internal bool System_ReadFile(struct Memory_Arena* arena, Path path, String* contents);
internal bool System_ResolvePath(struct Memory_Arena* arena, Path wd, Path path, Path* resolved_dir, Path* resolved_path);
internal bool System_PathsRefSameFile(Path p0, Path p1);

#include "mm_memory.h"

typedef struct Path_Label
{
    String label;
    Path path;
} Path_Label;

typedef struct MM_State
{
    union
    {
        struct
        {
            Memory_Arena misc_arena;
            Memory_Arena ast_arena;
            Memory_Arena intern_arena;
            Memory_Arena temp_arena;
            Memory_Arena check_arena;
        };
        
        Memory_Arena arena_bank[5];
    };
    
    File* first_file;
    File* last_file;
    File_ID next_file_id;
    
    Path_Label* path_labels;
    u32 path_label_count;
    
    struct AST_Node* ast;
    struct AST_Node* ast_last_node;
    
    struct AST_Node* checked_ast;
    struct AST_Node* checked_ast_last_node;
    
    bool encountered_errors;
    
    Interned_String keyword_strings[KEYWORD_COUNT];
    
    Interned_String intern_table[512];
    struct Symbol* global_symbol_table[512];
} MM_State;

global MM_State MM;

#include "mm_string.h"
#include "mm_bignum.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_symbols.h"
#include "mm_parser.h"
#include "mm_checker.h"

internal File*
MM_GetFile(File_ID id)
{
    File* file = MM.first_file;
    for (; file != 0; file = file->next)
    {
        if (file->id == id) break;
    }
    
    return file;
}

internal bool
MM_AddFile(String path, File_ID includer_id, u32 includer_offset, File_ID* id)
{
    bool encountered_errors = false;
    
    // NOTE: to simplify things, paths may only appear in one of these formats
    //       label:forward_slash_sepped_relative_path
    //       forward_slash_sepped_relative_path
    
    String label = {0};
    
    for (umm i = 0; i < path.size; ++i)
    {
        if (path.data[i] == ':')
        {
            label.data = path.data;
            label.size = i;
            
            path.data += i + 1;
            path.size -= i + 1;
            break;
        }
    }
    
    Path wd = {0};
    if (label.size == 0)
    {
        if (includer_id == 0) wd = MM.path_labels[0].path;
        else                  wd = MM_GetFile(includer_id)->dir;
    }
    
    else
    {
        umm i = 1;
        for (; i < MM.path_label_count; ++i)
        {
            if (String_Compare(label, MM.path_labels[i].label))
            {
                wd = MM.path_labels[i].path;
                break;
            }
        }
        
        if (i == MM.path_label_count)
        {
            //// ERROR: Invalid label name
            encountered_errors = true;
        }
    }
    
    Path resolved_path = {0};
    Path resolved_dir  = {0};
    if (!encountered_errors)
    {
        String ext = STRING(".m");
        for (umm i = 0; i < path.size; ++i)
        {
            if (path.data[i] == '.')
            {
                ext = (String){0};
                break;
            }
        }
        
        Memory_Arena_Marker marker = Arena_BeginTempMemory(&MM.temp_arena);
        if (ext.size != 0)
        {
            u8* memory = Arena_PushSize(&MM.temp_arena, path.size + ext.size, ALIGNOF(u8));
            
            Copy(path.data, memory, path.size);
            Copy(ext.data, memory + path.size, ext.size);
            memory[path.size + ext.size] = 0;
            
            path.data  = memory;
            path.size += ext.size;
        }
        
        if (!System_ResolvePath(&MM.misc_arena, wd, path, &resolved_dir, &resolved_path))
        {
            //// ERROR: Invalid file path
            encountered_errors = true;
        }
        
        else
        {
            for (File* scan = MM.first_file; scan != 0; scan = scan->next)
            {
                if (System_PathsRefSameFile(resolved_path, scan->path))
                {
                    //// ERROR: duplicate include
                    encountered_errors = true;
                    break;
                }
            }
        }
        
        Arena_EndTempMemory(&MM.temp_arena, marker);
    }
    
    if (!encountered_errors)
    {
        String contents;
        if (!System_ReadFile(&MM.misc_arena, resolved_path, &contents)) encountered_errors = true;
        else
        {
            File* file = Arena_PushSize(&MM.misc_arena, sizeof(File), ALIGNOF(File));
            
            *file = (File){
                .path     = resolved_path,
                .dir      = resolved_dir,
                .contents = contents,
                .id       = ++MM.next_file_id,
                
                .includer.id     = includer_id,
                .includer.offset = includer_offset,
            };
            
            if (id != 0) *id = file->id;
            
            if (MM.last_file) MM.last_file->next = file;
            else              MM.first_file      = file;
            
            MM.last_file = file;
            
            if (!ParseFile(file))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
MM_Init(String main_file_path, Path_Label* path_labels, u32 path_label_count)
{
    bool encountered_errors = false;
    
    ZeroStruct(&MM);
    
    /// Init memory
    for (umm i = 0; i < ARRAY_SIZE(MM.arena_bank); ++i)
    {
        System_InitArena(MM.arena_bank + i);
    }
    
    /// Init keyword lookup table
    String KeywordStrings[KEYWORD_COUNT] = {
        [Keyword_Do]             = STRING("do"),
        [Keyword_In]             = STRING("in"),
        [Keyword_Where]          = STRING("where"),
        [Keyword_Proc]           = STRING("proc"),
        [Keyword_Struct]         = STRING("struct"),
        [Keyword_Union]          = STRING("union"),
        [Keyword_Enum]           = STRING("enum"),
        [Keyword_True]           = STRING("true"),
        [Keyword_False]          = STRING("false"),
        [Keyword_As]             = STRING("as"),
        [Keyword_If]             = STRING("if"),
        [Keyword_Else]           = STRING("else"),
        [Keyword_When]           = STRING("when"),
        [Keyword_While]          = STRING("while"),
        [Keyword_For]            = STRING("for"),
        [Keyword_Break]          = STRING("break"),
        [Keyword_Continue]       = STRING("continue"),
        [Keyword_Using]          = STRING("using"),
        [Keyword_Defer]          = STRING("defer"),
        [Keyword_Return]         = STRING("return"),
        [Keyword_Include]        = STRING("include"),
    };
    
    for (umm i = Keyword_Invalid + 1; i < KEYWORD_COUNT; ++i)
    {
        MM.keyword_strings[i] = String_Intern(KeywordStrings[i]);
    }
    
    /// Misc. init
    MM.path_labels      = path_labels;
    MM.path_label_count = path_label_count;
    
    /// Parse main file
    if (!MM_AddFile(main_file_path, 0, 0, 0))
    {
        encountered_errors = true;
    }
    
    return !encountered_errors;
}