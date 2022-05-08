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

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u64 hash;
    u64 size;
} Interned_String_Entry;

typedef struct Workspace
{
    struct Arena* workspace_arena;
    struct Interned_String_Entry* intern_map[512];
    Interned_String_Entry intern_array[];
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

typedef u64 Interned_String;

#define INTERNED_STRING_NIL 0

#define LIST_KEYWORDS()             \
X(EMPTY_STRING,     "")         \
X(BLANK_IDENTIFIER, "_")        \
X(Keyword_Include,  "include")  \
X(Keyword_Proc,     "proc")     \
X(Keyword_Struct,   "struct")   \
X(Keyword_Union,    "union")    \
X(Keyword_Enum,     "enum")     \
X(Keyword_True,     "true")     \
X(Keyword_False,    "false")    \
X(Keyword_As,       "as")       \
X(Keyword_If,       "if")       \
X(Keyword_Else,     "else")     \
X(Keyword_While,    "while")    \
X(Keyword_Break,    "break")    \
X(Keyword_Continue, "continue") \
X(Keyword_Using,    "using")    \
X(Keyword_Defer,    "defer")    \
X(Keyword_Return,   "return")   \

enum
{
    Keyword_Dummy_Acc_Sentinel = sizeof(Interned_String_Entry),
    
#define X(e, s)                                                                                                     \
Keyword_Dummy_Acc_##e,                                                                                          \
e = ((Keyword_Dummy_Acc_##e - 1) + (sizeof(Interned_String_Entry) - 1)) & ~(sizeof(Interned_String_Entry) - 1), \
Keyword_Dummy_Pad_##e = e + sizeof(s) - 1,
    
    LIST_KEYWORDS()
        
#undef X
    
    Keyword_Dummy_Pad_Sentinel,
};

#if 0
internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** entry = &workspace->intern_map[hash % ARRAY_SIZE(workspace->intern_map)];
    
#if 0
    for (; *entry != 0; entry = &(*entry)->next)
    {
        if ((*entry)->hash == hash && String_Match(string, (String){ .data = (u8*)(*entry + 1), .size = (*entry)->size })) break;
    }
#else
    for (; *entry != 0 && !((*entry)->hash == hash && String_Match(string, (String){ .data = (u8*)(*entry + 1), .size = (*entry)->size })); entry = &(*entry)->next);
#endif
    
    return entry;
}
#else
internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** entry = &workspace->intern_map[hash % ARRAY_SIZE(workspace->intern_map)];
    
#if 0
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
    for (; *entry != 0 && ((*entry)->hash != hash || !String_Match(string, (String){ .data = (u8*)(*entry + 1), .size = (*entry)->size })); entry = &(*entry)->next);
#endif
    
    return entry;
}
#endif


internal Interned_String
InternedString__FromInternedStringEntry(Workspace* workspace, Interned_String_Entry* entry)
{
    return (Interned_String)(entry - workspace->intern_array);
}

internal Interned_String
InternedString_FromString(Workspace* workspace, String string)
{
    Interned_String_Entry** entry = InternedString__FindSpot(workspace, String_Hash(string), string);
    
    return (*entry == 0 ? INTERNED_STRING_NIL : *entry - workspace->intern_array);
}

internal String
InternedString_ToString(Workspace* workspace, Interned_String string)
{
    Interned_String_Entry* entry = workspace->intern_array + string;
    
    return (String){ .data = (u8*)(entry + 1), .size = entry->size };
}

internal bool
InternedString_IsKeyword(Interned_String string)
{
    return (string >= Keyword_Dummy_Acc_Sentinel && string < Keyword_Dummy_Pad_Sentinel);
}


#include "mm_bignum.h"
#include "mm_memory.h"
//#include "mm_lexer.h"

Workspace*
Workspace_Open()
{
    Arena* workspace_arena = Arena_Init(1);
    
    Workspace* workspace = Arena_PushSize(workspace_arena, sizeof(Workspace), ALIGNOF(Workspace));
    ZeroStruct(workspace);
    
    *workspace = (Workspace){
        .workspace_arena = workspace_arena,
    };
    
    Interned_String_Entry* nil_entry = Arena_PushSize(workspace->workspace_arena, sizeof(Interned_String_Entry), ALIGNOF(Interned_String_Entry));
    ZeroStruct(nil_entry);
    
#define X(e, s) Interned_String_Entry* e##_entry = Arena_PushSize(workspace->workspace_arena, sizeof(Interned_String_Entry) + sizeof(s) - 1, ALIGNOF(Interned_String_Entry)); \
u64 e##_hash = String_Hash(STRING(s));                                                                                                                                    \
Interned_String_Entry** e##_spot = InternedString__FindSpot(workspace, e##_hash, STRING(s));                                                                              \
ASSERT(*e##_spot == 0);                                                                                                                                                   \
*e##_spot = e##_entry;                                                                                                                                                    \
*e##_entry = (Interned_String_Entry){ .hash = e##_hash, .size = sizeof(s) - 1 };                                                                                          \
Copy(s, (u8*)(e##_entry + 1), sizeof(s) - 1);                                                                                                                             \
    
    LIST_KEYWORDS()
        
#undef X
    
    return workspace;
}

void
Workspace_Close(Workspace* workspace)
{
    Arena_Free(workspace->workspace_arena);
}

#endif