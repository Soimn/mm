#ifndef MM_HEADER
#define MM_HEADER

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifdef _WIN32
#ifndef _WIN64
#error 32 bit is not supported
#endif

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
#else
#error platform is not supported
#endif

typedef i64 imm;
typedef u64 umm;

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

umm   System_PageSize();
void* System_ReserveMemory(umm size);
void  System_CommitMemory(void* ptr, umm size);
void  System_FreeMemory(void* ptr);

typedef u64 Interned_String;
#define INTERNED_STRING_NIL 0
#define EMPTY_STRING 1
#define BLANK_IDENTIFIER 2
#define SPECIAL_STRING_COUNT 3

typedef enum KEYWORD_KIND
{
    Keyword_Include = SPECIAL_STRING_COUNT,
    Keyword_Proc,
    Keyword_Struct,
    Keyword_Union,
    Keyword_Enum,
    Keyword_True,
    Keyword_False,
    Keyword_As,
    Keyword_If,
    Keyword_Else,
    Keyword_While,
    Keyword_Break,
    Keyword_Continue,
    Keyword_Using,
    Keyword_Defer,
    Keyword_Return,
    
    KEYWORD_MAX
} KEYWORD_KIND;

#define KEYWORD_COUNT (KEYWORD_MAX - SPECIAL_STRING_COUNT)

typedef struct Workspace
{
    struct Arena* workspace_arena;
    struct Arena* string_arena;
    u64 const_string_thresh;
    String const_string_table[SPECIAL_STRING_COUNT + KEYWORD_COUNT];
    struct Interned_String_Entry* intern_table[512];
} Workspace;

internal u64
String_Hash(String string)
{
    // TODO: hash function
    return (string.size != 0 ? *string.data : 0);
}

internal bool
String_Match(String s0, String s1)
{
    bool result = (s0.size == s1.size);
    
    for (umm i = 0; i < s0.size && result; ++i)
    {
        result = (s0.data[i] == s1.data[i]);
    }
    
    return result;
}

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u64 hash;
    u64 size;
    u8 data[];
} Interned_String_Entry;

internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** entry = &workspace->intern_table[hash % ARRAY_SIZE(workspace->intern_table)];
    
#if 1
    for (; *entry != 0; entry = &(*entry)->next)
    {
        if ((*entry)->hash != hash) continue;
        else
        {
            String entry_string = {
                .data = (*entry)->data,
                .size = (*entry)->size,
            };
            
            if (String_Match(string, entry_string)) break;
        }
    }
#else
    for (; *entry != 0 && ((*entry)->hash != hash || !String_Match(string, (String){ .data = (*entry)->data, .size = (*entry)->size })); entry = &(*entry)->next);
#endif
    
    return entry;
}

internal Interned_String
InternedString_GetFromString(Workspace* workspace, String string)
{
    u64 hash = String_Hash(string);
    Interned_String_Entry** entry = InternedString__FindSpot(workspace, hash, string);
    
    Interned_String result = (Interned_String)*entry;
    if (*entry != 0 && (umm)*entry < workspace->const_string_thresh)
    {
        ASSERT((umm)workspace->string_arena <= (umm)*entry);
        
        result = (Interned_String)(*entry)->data[(*entry)->size];
        
        ASSERT((umm)result < KEYWORD_MAX);
    }
    
    return result;
}

internal bool
InternedString_IsKeyword(Interned_String string)
{
    return ((umm)string >= SPECIAL_STRING_COUNT && (umm)string < KEYWORD_MAX);
}

#include "mm_bignum.h"
#include "mm_memory.h"
#include "mm_lexer.h"

Workspace*
Workspace_Open()
{
    Arena* workspace_arena = Arena_Init(1);
    Arena* string_arena    = Arena_Init(2);
    
    Workspace* workspace = Arena_PushSize(workspace_arena, sizeof(Workspace), ALIGNOF(Workspace));
    ZeroStruct(workspace);
    
    *workspace = (Workspace){
        .workspace_arena = workspace_arena,
        .string_arena    = string_arena,
        .const_string_table = {
            [INTERNED_STRING_NIL] = {0},
            [EMPTY_STRING]        = STRING(""),
            [BLANK_IDENTIFIER]    = STRING("_"),
            [Keyword_Include]     = STRING("include"),
            [Keyword_Proc]        = STRING("proc"),
            [Keyword_Struct]      = STRING("struct"),
            [Keyword_Union]       = STRING("union"),
            [Keyword_Enum]        = STRING("enum"),
            [Keyword_True]        = STRING("true"),
            [Keyword_False]       = STRING("false"),
            [Keyword_As]          = STRING("as"),
            [Keyword_If]          = STRING("if"),
            [Keyword_Else]        = STRING("else"),
            [Keyword_While]       = STRING("while"),
            [Keyword_Break]       = STRING("break"),
            [Keyword_Continue]    = STRING("continue"),
            [Keyword_Using]       = STRING("using"),
            [Keyword_Defer]       = STRING("defer"),
            [Keyword_Return]      = STRING("return"),
        },
    };
    
    for (umm i = 1; // NOTE: skip nil string
         i < ARRAY_SIZE(workspace->const_string_table);
         ++i)
    {
        String string = workspace->const_string_table[i];
        u64 hash      = String_Hash(string);
        Interned_String_Entry** entry = InternedString__FindSpot(workspace, hash, string);
        
        *entry = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry) + string.size + 1, ALIGNOF(Interned_String_Entry));
        **entry = (Interned_String_Entry){
            .next   = 0,
            .hash   = hash,
            .size   = string.size,
        };
        
        Copy(string.data, (*entry)->data, string.size);
        (*entry)->data[string.size] = (u8)i; // NOTE: Const strings store their id in a byte right after the string
    }
    
    workspace->const_string_thresh = (u64)((u8*)(string_arena + 1) + string_arena->offset);
    
    return workspace;
}

void
Workspace_Close(Workspace* workspace)
{
    Arena_Free(workspace->string_arena);
    
    Arena_Free(workspace->workspace_arena);
}

#endif