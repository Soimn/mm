#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifndef _WIN64
#error 32 bit mode is not supported yet
#endif

#include <immintrin.h>

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

typedef float  f32;
typedef double f64;

typedef u8 bool;

typedef struct String
{
    u8* data;
    u64 size;
} String;

#define STRING(str) (String){ .data = (u8*)(str), .size = sizeof(str) - 1 }

#define true 1
#define false 0

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

#if MM_DEBUG
#define ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0))
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

#define IS_POW_OF_2(n) (((n == 0) | ((n) & ((n) - 1))) == 0)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define internal static
#define global static

// NOTE: This is just a hack to work around a parsing bug in 4coder
#define TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

typedef u32 Type_ID;
typedef u32 File_ID;

enum KEYWORD_KIND
{
    Keyword_Include = 0,
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
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    
    KEYWORD_KIND_MAX = Keyword_Return,
    KEYWORD_KIND_COUNT,
};

#define IS_KEYWORD(istring) ((istring) <= KEYWORD_KIND_MAX)

typedef u64 Interned_String;
#define BLANK_IDENTIFIER U64_MAX

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u32 hash;
    u32 size;
} Interned_String_Entry;

typedef u64 File_Handle;

struct Arena;
internal inline void* System_ReserveMemory(umm size);
internal inline bool System_CommitMemory(void* ptr, umm size);
internal inline void System_FreeMemory(void* ptr);
internal bool System_OpenFile(String path, File_Handle* handle);
internal bool System_ReadFile(File_Handle handle, struct Arena* arena, String* string);
internal bool System_FileHandlesAreEqual(File_Handle a, File_Handle b);
internal void System_CloseFile(File_Handle handle);

typedef struct File
{
    struct File* next;
    File_Handle handle;
} File;

typedef struct MM_State
{
    struct Arena* file_arena;
    struct Arena* misc_arena;
    struct Arena* ast_arena;
    struct Arena* string_arena;
    
    File* first_file;
    File** next_file;
    struct AST_Node* first_parsed_ast;
    struct AST_Node** next_parsed_ast;
    u64 keyword_threshold;
    
    Interned_String_Entry* string_table[512];
    Interned_String_Entry* keyword_table[KEYWORD_KIND_COUNT];
} MM_State;

MM_State MM = {0};

#include "mm_memory.h"
#include "mm_string.h"

internal inline Interned_String_Entry**
MM_GetInternedStringSlot(String string, u32* hash)
{
    *hash = String_HashOf(string) & ~(u32)0;
    
    Interned_String_Entry** slot = &MM.string_table[*hash % ARRAY_SIZE(MM.string_table)];
    
    for (; *slot != 0; slot = &(*slot)->next)
    {
        if ((*slot)->hash == *hash && String_Match((String){(u8*)(*slot + 1), (*slot)->size}, string))
        {
            break;
        }
    }
    
    return slot;
}

internal Interned_String
MM_InternString(String string)
{
    u32 hash;
    Interned_String_Entry** slot = MM_GetInternedStringSlot(string, &hash);
    
    if (*slot == 0)
    {
        *slot = Arena_PushSize(MM.string_arena, sizeof(Interned_String_Entry) + string.size, ALIGNOF(Interned_String_Entry));
        
        **slot = (Interned_String_Entry){
            .next = 0,
            .hash = hash,
            .size = (u32)string.size,
        };
        
        Copy(string.data, *slot + 1, string.size);
    }
    
    Interned_String result;
    if ((u64)*slot < MM.keyword_threshold) result = *((u8*)(*slot + 1) + (*slot)->size);
    else                                   result = (u64)*slot;
    
    return result;
}

#include "mm_bignum.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"
#include "mm_types.h"
#include "mm_const_val.h"
#include "mm_symbols.h"
#include "mm_checker.h"

internal bool
MM_Init()
{
    bool encountered_errors = false;
    
    ZeroStruct(&MM);
    
    MM.file_arena   = Arena_Init(GB(8));
    MM.misc_arena   = Arena_Init(GB(8));
    MM.ast_arena    = Arena_Init(GB(8));
    MM.string_arena = Arena_Init(GB(8));
    
    String keywords[KEYWORD_KIND_COUNT] = {
        [Keyword_Include]  = STRING("include"),
        [Keyword_Proc]     = STRING("proc"),
        [Keyword_Struct]   = STRING("struct"),
        [Keyword_Union]    = STRING("union"),
        [Keyword_Enum]     = STRING("enum"),
        [Keyword_True]     = STRING("true"),
        [Keyword_False]    = STRING("false"),
        [Keyword_As]       = STRING("as"),
        [Keyword_If]       = STRING("if"),
        [Keyword_Else]     = STRING("else"),
        [Keyword_When]     = STRING("when"),
        [Keyword_While]    = STRING("while"),
        [Keyword_Break]    = STRING("break"),
        [Keyword_Continue] = STRING("continue"),
        [Keyword_Using]    = STRING("using"),
        [Keyword_Defer]    = STRING("defer"),
        [Keyword_Return]   = STRING("return"),
    };
    
    for (umm i = 0; i < KEYWORD_KIND_COUNT; ++i)
    {
        u32 hash;
        Interned_String_Entry** slot = MM_GetInternedStringSlot(keywords[i], &hash);
        
        ASSERT(*slot == 0);
        
        *slot = Arena_PushSize(MM.string_arena,
                               sizeof(Interned_String_Entry) + keywords[i].size + 1,
                               ALIGNOF(Interned_String_Entry));
        
        **slot = (Interned_String_Entry){
            .next = 0,
            .hash = hash,
            .size = (u32)keywords[i].size,
        };
        
        Copy(keywords[i].data, *slot + 1, keywords[i].size);
        
        // NOTE: Keywords store their keyword index right after the character data
        *((u8*)(*slot + 1) + keywords[i].size) = (u8)i;
        
        
        MM.keyword_table[i] = *slot;
    }
    
    MM.keyword_threshold = (u64)(MM.string_arena->base + MM.string_arena->offset);
    
    // NOTE: Initializing singly linked lists
    MM.next_file       = &MM.first_file;
    MM.next_parsed_ast = &MM.first_parsed_ast;
    
    return !encountered_errors;
}

internal bool
MM_IncludeFile(String path)
{
    bool encountered_errors = false;
    
    File_Handle handle;
    if (!System_OpenFile(path, &handle))
    {
        //// ERROR: Failed to open file
        encountered_errors = true;
    }
    else
    {
        File* file = MM.first_file;
        for (; file != 0; file = file->next)
        {
            if (System_FileHandlesAreEqual(handle, file->handle))
            {
                break;
            }
        }
        
        if (file != 0) System_CloseFile(handle); // NOTE: files that are stored are not closed
        else
        {
            *MM.next_file = Arena_PushSize(MM.file_arena, sizeof(File), ALIGNOF(File));
            
            **MM.next_file = (File){
                .next   = 0,
                .handle = handle,
            };
            
            Arena_Marker marker = Arena_BeginTemp(MM.file_arena);
            
            String file_contents;
            if (!System_ReadFile(handle, MM.file_arena, &file_contents))
            {
                //// ERROR: Failed to read file
                encountered_errors = true;
            }
            else
            {
                if (!ParseString(file_contents, MM.ast_arena, MM.next_parsed_ast))
                {
                    encountered_errors = true;
                }
            }
            
            Arena_EndTemp(MM.file_arena, marker);
        }
    }
    
    return !encountered_errors;
}

typedef enum COMPILER_MESSAGE_KIND
{
    CompilerMessage_LoadedFile,
    CompilerMessage_ParsedFile,
    CompilerMessage_CheckedDecl,
    
    CompilerMessage_Error,
    CompilerMessage_Warning,
} COMPILER_MESSAGE_KIND;

typedef struct Compiler_Message
{
    COMPILER_MESSAGE_KIND kind;
    
    union
    {
        struct
        {
            // filename, path
            // contents
            // who asked for this file
        } loaded_file;
        
        struct
        {
            // which file
            // ast
        } parsed_file;
        
        struct
        {
            // which file
            // ast
            // type information
        } checked_decl;
    };
} Compiler_Message;