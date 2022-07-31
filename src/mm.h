#ifndef MM_H
#define MM_H

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

/// Basic Types
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
typedef signed __int8  MM_i8;
typedef signed __int16 MM_i16;
typedef signed __int32 MM_i32;
typedef signed __int64 MM_i64;
typedef signed __int128 MM_i128;

typedef unsigned __int8  MM_u8;
typedef unsigned __int16 MM_u16;
typedef unsigned __int32 MM_u32;
typedef unsigned __int64 MM_u64;
typedef unsigned __int128 MM_u128;
#else
#error "Platform is not supported"
#endif

// NOTE: umm and imm are used instead of uint and int. They are essentially
//       unsigned/signed 64 bit integers used for computation
//       "where the width does not matter", but it should be "wide enough"
typedef MM_u64 MM_umm;
typedef MM_i64 MM_imm;

typedef MM_u16 MM_f16;
typedef float  MM_f32;
typedef double MM_f64;

typedef MM_u8 MM_bool;

#define MM_true (MM_bool)(1)
#define MM_false (MM_bool)(0)

#define MM_U8_MAX   (MM_u8)  0xFF
#define MM_U16_MAX  (MM_u16) 0xFFFF
#define MM_U32_MAX  (MM_u32) 0xFFFFFFFF
#define MM_U64_MAX  (MM_u64) 0xFFFFFFFFFFFFFFFF
#define MM_U128_MAX (MM_u128)((MM_u128)MM_U64_MAX << 64 | MM_U64_MAX)

#define MM_I8_MIN   (MM_i8)  0x80
#define MM_I16_MIN  (MM_i16) 0x8000
#define MM_I32_MIN  (MM_i32) 0x80000000
#define MM_I64_MIN  (MM_i64) 0x8000000000000000
#define MM_I128_MIN (MM_i128)((MM_u128)MM_I64_MIN << 64)

#define MM_I8_MAX   (MM_i8)  0x7F
#define MM_I16_MAX  (MM_i16) 0x7FFF
#define MM_I32_MAX  (MM_i32) 0x7FFFFFFF
#define MM_I64_MAX  (MM_i64) 0x7FFFFFFFFFFFFFFF
#define MM_I128_MAX (MM_i128)((MM_u128)MM_I64_MAX << 64 | MM_U64_MAX)

#define MM__CONCAT(x, y) x##y
#define MM_CONCAT(x, y) MM__CONCAT(x, y)

#define MM_STATIC_ASSERT(EX) struct MM_CONCAT(MM_STATIC_ASSERT_, MM_CONCAT(__COUNTER__, MM_CONCAT(_, __LINE__))) { int a : (EX) ? 1 : -1; }

#if MM_DEBUG
#define MM_ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0))
#else
#define MM_ASSERT(EX) (void)(EX)
#endif

#define MM_NOT_IMPLEMENTED MM_ASSERT(!"NOT_IMPLEMENTED")
#define MM_INVALID_DEFAULT_CASE default: MM_ASSERT(!"INVALID_DEFAULT_CASE"); break
#define MM_INVALID_CODE_PATH MM_ASSERT(!"INVALID_CODE_PATH")

#define MM_OFFSETOF(element, type) (MM_umm)&((type*)0)->element
#define MM_ALIGNOF(T) MM_OFFSETOF(t, struct { MM_u8 b; T t; })

#define MM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MM_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MM_ABS(n) ((n) < 0 ? -(n) : (n))
#define MM_SGN(n) ((n) < 0 ? -1 : ((n) == 0 ? 0 : 1))

#define MM_KB(N) ((MM_umm)(N) << 10)
#define MM_MB(N) ((MM_umm)(N) << 20)
#define MM_GB(N) ((MM_umm)(N) << 30)
#define MM_TB(N) ((MM_umm)(N) << 40)

#define MM_IS_POW_OF_2(n) (((n == 0) | ((n) & ((n) - 1))) == 0)

#define MM_ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define MM_TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

typedef struct MM_String
{
    MM_u8* data;
    MM_u64 size;
} MM_String;

#define MM_STRING(S) (MM_String){ .data = (MM_u8*)(S), .size = sizeof(S) - 1 }

typedef struct MM_Slice
{
    void* data;
    MM_u64 size;
} MM_Slice;

#define MM_Slice(T) MM_Slice

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MM_Lexer MM_Lexer_Init(MM_String string);
struct MM_Token MM_Lexer_CurrentToken(struct MM_Lexer* lexer);
struct MM_Token MM_Lexer_NextToken(struct MM_Lexer* lexer);
MM_umm MM_Lexer_NextTokens(struct MM_Lexer* lexer, struct MM_Token* buffer, MM_umm buffer_size);

// TODO:
typedef MM_String MM_Identifier;
typedef MM_String MM_String_Literal;
typedef MM_f64 MM_Soft_Float;
//

// TODO: How should Out of Memory be handled since this is a library?
void* MM_System_DefaultReserveMemory(MM_umm size);
void MM_System_DefaultCommitMemory(void* ptr, MM_umm size);
void MM_System_DefaultFreeMemory(void* ptr);

#ifndef MM_SYSTEM_PAGE_SIZE
#define MM_SYSTEM_PAGE_SIZE MM_KB(4)
#endif

#ifndef MM_SYSTEM_RESERVE_BLOCK_SIZE
#define MM_SYSTEM_RESERVE_BLOCK_SIZE MM_KB(64)
#endif

#ifndef MM_SYSTEM_RESERVE_MEMORY
#define MM_SYSTEM_RESERVE_MEMORY(size) MM_System_DefaultReserveMemory(size)
#endif

#ifndef MM_SYSTEM_COMMIT_MEMORY
#define MM_SYSTEM_COMMIT_MEMORY(ptr, size) MM_System_DefaultCommitMemory((ptr), (size))
#endif

#ifndef MM_SYSTEM_FREE_MEMORY
#define MM_SYSTEM_FREE_MEMORY(ptr) MM_System_DefaultFreeMemory(ptr)
#endif
typedef enum MM_ENTITY_KIND
{
    MM_Entity_Variable,
    MM_Entity_Constant,
    MM_Entity_WhenDecl,
    MM_Entity_IncludeDecl,
} MM_ENTITY_KIND;

typedef struct MM_Entity
{
    MM_ENTITY_KIND kind;
    struct MM_AST* ast;
    // TODO: tracking, history, etc.
} MM_Entity;

typedef struct MM_Workspace
{
    struct MM_Arena* workspace_arena;
    struct MM_Arena* ast_arena;
    struct MM_String_Intern_Table* intern_table;
} MM_Workspace;

#ifdef _WIN32
#include "mm_win32.h"
#endif

#include "mm_memory.h"
#include "mm_string.h"
#include "mm_int.h"
#include "mm_float.h"
#include "mm_workspace.h"
#include "mm_lexer.h"
#include "mm_ast.h"
#include "mm_parser.h"

#endif