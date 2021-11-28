#include <stdarg.h>

#ifndef IZ_DEBUG
#define IZ_DEBUG 0
#endif

#ifndef IZ_INTERNAL
#define IZ_INTERNAL IZ_DEBUG
#endif

#ifdef _WIN32

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

#pragma function(memset)

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


int _fltused;

int __stdcall
_DllMainCRTStartup(void* instance, u32 reason, void* reserved)
{
    return 1;
}

#elif __linux__

typedef __INT8_TYPE__  i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef __UINT8_TYPE__  u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;

#ifdef _LP64
typedef u64 umm;
typedef i64 imm;
#else
typedef u32 umm;
typedef i32 imm;
#endif

#endif

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

#if IZ_INTERNAL || IZ_DEBUG
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


typedef String Identifier;
typedef String String_Literal;

typedef struct Number
{
    union
    {
        u64 integer;
        f32 float32;
        f64 float64;
    };
    
    bool is_float32;
    bool is_float64;
    bool is_negative;
} Number;

typedef union Character
{
    u32 word;
    u8 bytes[4];
} Character;

#include "mm_memory.h"
#include "mm_string.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"