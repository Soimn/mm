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

typedef struct Cap_Buffer
{
    u8* data;
    u64 size;
    u64 capacity;
} Cap_Buffer;

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

#define BLANK_IDENTIFIER (Identifier){0}

typedef struct Number
{
    union
    {
        u64 integer;
        f64 floating;
    };
    
    bool is_float;
    bool is_negative;
    u8 width;
} Number;

typedef union Character
{
    u32 word;
    u8 bytes[4];
} Character;

typedef u64 Package_ID;
typedef u64 File_ID;
typedef u32 Type_ID;

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
    
    KEYWORD_COUNT,
};

internal Identifier Identifier_Add(String string);
internal String Identifier_ToString(Identifier identifier);
internal inline bool Identifier_IsKeyword(Identifier identifier, Enum8(KEYWORD_KIND) keyword);

#include "mm_memory.h"
#include "mm_string.h"
#include "mm_lexer.h"
#include "mm_symbols.h"
#include "mm_ast.h"
#include "mm_parser.h"
#include "mm_checker.h"
#include "mm_code_gen.h"

typedef struct MM_State
{
    Memory_Arena string_arena;
    
    Package* packages;
    u32 package_count;
    
    File* files;
    u32 file_count;
    
    String* identifier_table;
    u32 identifier_table_size;
    
    Type_Info* type_table;
    u32 type_table_size;
    
    Identifier keyword_identifiers[KEYWORD_COUNT];
} MM_State;

global MM_State MM;

internal Identifier
Identifier_Add(String string)
{
    ASSERT(string.data != 0 && string.size != 0);
    
    // HACK
    // TODO: Implement a hash table with stable pointers
    
    Identifier result = BLANK_IDENTIFIER;
    
    imm free = -1;
    umm i    = 0;
    for (; i < MM.identifier_table_size; ++i)
    {
        if (String_Compare(string, MM.identifier_table[i]))
        {
            result = (Identifier)i;
            break;
        }
        
        else if (MM.identifier_table[i].data == 0) free = (imm)i;
    }
    
    // NOTE: no existing entry was found, add a new entry
    if (i == MM.identifier_table_size)
    {
        // TODO: grow table
        ASSERT(free != -1);
        
        void* memory = Arena_PushSize(&MM.string_arena, string.size, 1);
        Copy(string.data, memory, string.size);
        
        MM.identifier_table[free] = (String){
            .data = memory,
            .size = string.size
        };
        
        result = (Identifier)free;
    }
    
    return result;
}

internal String
Identifier_ToString(Identifier identifier)
{
    return MM.identifier_table[identifier];
}

internal inline bool
Identifier_IsKeyword(Identifier identifier, Enum8(KEYWORD_KIND) keyword)
{
    return (MM.keyword_identifiers[keyword] == identifier);
}

internal inline Type_Info*
TypeInfo_FromTypeID(Type_ID id)
{
    return &MM.type_table[id];
}

internal Type_ID
TypeInfo_Add(Type_Info info)
{
    Type_ID id = {0};
    
    NOT_IMPLEMENTED;
    
    return id;
}


internal void
MM_Init()
{
    ZeroStruct(&MM);
    
    /// Init memory
    NOT_IMPLEMENTED;
    
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
        [Keyword_Import]         = STRING("import"),
        [Keyword_Foreign]        = STRING("foreign"),
    };
    
    for (umm i = Keyword_Invalid + 1; i < KEYWORD_COUNT; ++i)
    {
        MM.keyword_identifiers[i] = Identifier_Add(KeywordStrings[i]);
    }
    
    /// Init type table
    
    // allocate type table
    NOT_IMPLEMENTED;
    
    for (Type_ID i = Type_FirstUntyped;
         i <= Type_LastUntyped;
         ++i)
    {
        MM.type_table[i] = (Type_Info){ .kind = (u8)i };
    }
    
#define TYPED_BASE_TYPE(id, type, ident)            \
MM.type_table[id] = (Type_Info){                \
.kind      = id,                            \
.size      = sizeof(type),                  \
.alignment = ALIGNOF(type),                 \
.name      = Identifier_Add(STRING(ident)), \
}
    
    TYPED_BASE_TYPE(Type_String, String, "string");
    TYPED_BASE_TYPE(Type_Char,   u32,    "char");
    TYPED_BASE_TYPE(Type_Bool,   u8,     "bool");
    TYPED_BASE_TYPE(Type_Int,    i64,    "int");
    TYPED_BASE_TYPE(Type_I8,     i8,     "i8");
    TYPED_BASE_TYPE(Type_I16,    i16,    "i16");
    TYPED_BASE_TYPE(Type_I32,    i32,    "i32");
    TYPED_BASE_TYPE(Type_I64,    i64,    "i64");
    TYPED_BASE_TYPE(Type_Uint,   u64,    "uint");
    TYPED_BASE_TYPE(Type_U8,     u8,     "u8");
    TYPED_BASE_TYPE(Type_U16,    u16,    "u16");
    TYPED_BASE_TYPE(Type_U32,    u32,    "u32");
    TYPED_BASE_TYPE(Type_U64,    u64,    "u64");
    TYPED_BASE_TYPE(Type_Float,  f64,    "f64");
    TYPED_BASE_TYPE(Type_F32,    f32,    "f32");
    TYPED_BASE_TYPE(Type_F64,    f64,    "f64");
}