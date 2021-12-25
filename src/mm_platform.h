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

#define STRING(str) (String){ .data = (u8*)(str), .size = sizeof(str) - 1 }

typedef struct File_Buffer
{
    void* memory;
    u32 size;
    u32 offset;
    bool written_past_end;
} File_Buffer;

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

typedef u32 Identifier;
typedef String String_Literal;

#define BLANK_IDENTIFIER 0

typedef struct Number
{
    union
    {
        u64 integer;
        f64 floating;
    };
    
    bool is_float;
} Number;

typedef union Character
{
    u32 word;
    u8 bytes[4];
} Character;

typedef i32 Package_ID;
typedef i32 File_ID;
typedef u32 Type_ID;

#define INVALID_PACKAGE_ID -1
#define INVALID_FILE_ID -1

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
    Keyword_Import,
    Keyword_Foreign,
    Keyword_Package,
    
    KEYWORD_COUNT,
};

// NOTE: This is just a hack to work around a parsing bug in 4coder
struct Symbol;
typedef struct Symbol* Symbol_Table_;
typedef Symbol_Table_ Symbol_Table;

typedef struct File
{
    struct AST_Node* ast;
    String file_name;
} File;

typedef struct Package
{
    String path;
    File* files;
    u32 file_count;
    Identifier name;
    Symbol_Table symbol_table;
} Package;

#include "mm_memory.h"
#include "mm_string.h"

typedef struct MM_State
{
    Memory_Arena temp_arena;
    Memory_Arena string_arena;
    
    Package* packages;
    u32 package_count;
    
    File* files;
    u32 file_count;
    
    String* identifier_table;
    u32 identifier_table_size;
    
    struct Type_Info* type_table;
    u32 type_table_size;
    
    Identifier keyword_identifiers[KEYWORD_COUNT];
} MM_State;

global MM_State MM;

internal Identifier Identifier_Add(String string);
internal String Identifier_ToString(Identifier identifier);
internal inline bool Identifier_IsKeyword(Identifier identifier, Enum8(KEYWORD_KIND) keyword);
internal inline Package* Package_FromID(Package_ID id);

#include "mm_lexer.h"
#include "mm_symbols.h"
#include "mm_ast.h"
#include "mm_parser.h"
#include "mm_checker.h"
#include "mm_code_gen.h"
#include "mm.h"