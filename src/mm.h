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

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define internal static
#define global static

// NOTE: This is just a hack to work around a parsing bug in 4coder
#define TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

enum KEYWORD_KIND
{
    Keyword_Include,
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

typedef u64 Interned_String;
#define BLANK_IDENTIFIER U64_MAX

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u32 hash;
    u32 size;
} Interned_String_Entry;

internal inline void* System_ReserveMemory(umm size);
internal inline bool System_CommitMemory(void* ptr, umm size);
internal inline void System_FreeMemory(void* ptr, umm size);

typedef struct MM_State
{
    struct Arena* misc_arena;
    struct Arena* ast_arena;
    struct Arena* string_arena;
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

#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"

internal bool
MM_Init()
{
    bool encountered_errors = false;
    
    MM.ast_arena    = Arena_Init();
    MM.string_arena = Arena_Init();
    
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
        
        // NOTE: Keywords stor their keyword index right after the character data
        *((u8*)(*slot + 1) + keywords[i].size) = (u8)i;
        
        
        MM.keyword_table[i] = *slot;
    }
    
    MM.keyword_threshold = (u64)(MM.string_arena->base + MM.string_arena->offset);
    
    return !encountered_errors;
}

internal String
MM_ResolvePath(String path, Arena* arena)
{
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
    
    String resolved_path = {0};
    if (label.data == 0)
    {
        resolved_path.size = path.size;
        resolved_path.data = Arena_PushSize(MM.string_arena, resolved_path.size, ALIGNOF(u8));
        
        Copy(path.data, resolved_path.data, reolved_path.size);
    }
    else
    {
        NOT_IMPLEMENTED;
    }
    
    return resolved_path;
}