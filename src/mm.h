#ifndef MM_HEADER
#define MM_HEADER

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifndef _WIN64
#error 32 bit is not supported
#endif

#include <intrin.h>

typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float  f32;
typedef double f64;

typedef i64 imm;
typedef u64 umm;

typedef u8 bool;

typedef struct String
{
    u8* data;
    u64 size;
} String;

typedef String ZString; // NOTE: Zero terminated String (null is not included in .size)

#define STRING(str) (String){ .data = (u8*)(str), .size = sizeof(str) - 1 }

#define true 1
#define false 0

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define I8_MIN  0x80
#define I16_MIN 0x8000
#define I32_MIN 0x80000000
#define I64_MIN 0x8000000000000000

#define I8_MAX  0x7F
#define I16_MAX 0x7FFF
#define I32_MAX 0x7FFFFFFF
#define I64_MAX 0x7FFFFFFFFFFFFFFF

#if MM_DEBUG
#define ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0))
#else
#define ASSERT(EX) (void)(EX)
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

#define IS_POW_OF_2(n) (((n == 0) | ((n) & ((n) - 1))) == 0)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define internal static
#define global static

// NOTE: This is just a hack to work around a parsing bug in 4coder
#define TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

TYPEDEF_FUNC(void*, system_reserve_memory_func, umm size);
TYPEDEF_FUNC(void, system_commit_memory_func, void* ptr, umm size);
TYPEDEF_FUNC(void, system_free_memory_func, void* ptr);
TYPEDEF_FUNC(void, system_print_func, ZString string);

typedef u64 File_ID;

typedef struct File
{
    File_ID id;
    String path;
    ZString content;
} File;

typedef struct Text_Pos
{
    u32 offset;
    u32 line;
    u32 column;
} Text_Pos;

typedef struct Text_Interval
{
    struct Text_Pos;
    u32 size;
} Text_Interval;

internal Text_Interval
TextInterval_BetweenStartPoints(Text_Interval included, Text_Interval excluded)
{
    ASSERT(included.line == excluded.line);
    ASSERT(included.offset + included.column <= excluded.offset + excluded.column);
    
    return (Text_Interval){
        .offset = included.offset,
        .line   = included.line,
        .column = included.column,
        .size   = (excluded.offset + excluded.column) - (included.offset - included.column),
    };
}

internal Text_Interval
TextInterval_ContainingBoth(Text_Interval a, Text_Interval b)
{
    ASSERT(a.line == b.line);
    ASSERT(a.offset + a.column + a.size < b.offset);
    
    return (Text_Interval){
        .offset = a.offset,
        .line   = a.line,
        .column = a.column,
        .size   = (b.offset + b.column + b.size) - (a.offset - a.column),
    };
}

#define ERROR_CODE_NIL 0
#define LEXER_ERROR_CODE_BASE  (1 << 10)
#define PARSER_ERROR_CODE_BASE (1 << 11)

typedef u32 Error_Code;

typedef struct Error_Report
{
    Error_Code code;
    String message;
    Text_Interval text;
    File_ID file_id;
} Error_Report;

typedef struct Workspace
{
    struct Arena* workspace_arena;
    struct Arena* ast_arena;
    struct Arena* ast_info_arena;
    struct Arena* string_arena;
    struct Arena* file_arena;
    
    struct AST_Node* head_ast;
    struct AST_Node** tail_ast;
    
    Error_Report report;
} Workspace;

internal File*
File_FromFileID(Workspace* workspace, File_ID file_id)
{
    return (File*)file_id;
}

#include "mm_bignum.h"
#include "mm_memory.h"
#include "mm_string.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"

typedef struct Workspace_Options
{
    system_reserve_memory_func ReserveMemory;
    system_commit_memory_func CommitMemory;
    system_free_memory_func FreeMemory;
    u64 page_size;
} Workspace_Options;

Workspace*
Workspace_Open(Workspace_Options options)
{
    Arena* workspace_arena = Arena_Init(options.ReserveMemory, options.CommitMemory, options.FreeMemory, options.page_size);
    Arena* ast_arena       = Arena_Init(options.ReserveMemory, options.CommitMemory, options.FreeMemory, options.page_size);
    Arena* ast_info_arena  = Arena_Init(options.ReserveMemory, options.CommitMemory, options.FreeMemory, options.page_size);
    Arena* string_arena    = Arena_Init(options.ReserveMemory, options.CommitMemory, options.FreeMemory, options.page_size);
    Arena* file_arena      = Arena_Init(options.ReserveMemory, options.CommitMemory, options.FreeMemory, options.page_size);
    
    Workspace* workspace = Arena_PushSize(workspace_arena, sizeof(Workspace), ALIGNOF(Workspace));
    ZeroStruct(workspace);
    
    *workspace = (Workspace){
        .workspace_arena = workspace_arena,
        .ast_arena       = ast_arena,
        .ast_info_arena  = ast_info_arena,
        .string_arena    = string_arena,
        .file_arena      = file_arena,
    };
    
    workspace->tail_ast = &workspace->head_ast;
    
    InternedString__Init(workspace);
    
    return workspace;
}

void
Workspace_Close(Workspace* workspace)
{
    Arena_Free(workspace->workspace_arena);
}

bool
Workspace_HasErrors(Workspace* workspace)
{
    return workspace->report.code != 0;
}

void
Workspace_PrintErrors(Workspace* workspace, system_print_func Print)
{
    Arena_Marker marker = Arena_BeginTemp(workspace->workspace_arena);
    
    Error_Report* report = &workspace->report;
    File* file           = File_FromFileID(workspace, report->file_id);
    
    if (report->code != 0)
    {
        Print(String_Printf(workspace->workspace_arena, "%S(%u,%u): error C%u: %S\n\0", file->path, report->text.line, report->text.column, report->code, report->message));
    }
    
    Arena_EndTemp(workspace->workspace_arena, marker);
}

#endif