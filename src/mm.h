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
    Arena string_arena;
    Interned_String_Entry* string_table[512];
} MM_State;

#include "mm_memory.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"