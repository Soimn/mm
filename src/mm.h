#ifndef MM_HEADER
#define MM_HEADER

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endi

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

typedef enum KEYWORD_KIND
{
    Keyword_,
    
    KEYWORD_COUNT
} KEYWORD_KIND;

typedef struct Workspace
{
    Arena* string_arena;
    u64 intern_const_thresh;
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
    NOT_IMPLEMENTED;
}

typedef void* Interned_String;
#define INTERNED_STRING_NIL 0

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u64 hash;
    u64 size;
} Interned_String_Entry;

internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** entry = &workspace->intern_table[hash % sizeof(workspace->intern_table)];
    
    for (; *entry != 0 && (*entry)->hash != hash && !String_Match(string, (String){ .data = (u8*)(*entry + 1), .size = (*entry)->size }); entry = &(*entry)->next);
    
    return entry;
}

#include "mm_bignum.h"
#include "mm_memory.h"
#include "mm_lexer.h"

#endif