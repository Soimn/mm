#ifndef MM_H
#define MM_H

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#if _WIN32
typedef signed __int8   MM_i8;
typedef signed __int16  MM_i16;
typedef signed __int32  MM_i32;
typedef signed __int64  MM_i64;
typedef signed __int128 MM_i128;

typedef unsigned __int8   MM_u8;
typedef unsigned __int16  MM_u16;
typedef unsigned __int32  MM_u32;
typedef unsigned __int64  MM_u64;
typedef unsigned __int128 MM_u128;
#else
#error NOT_IMPLEMENTED
#endif

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

// TODO: check this is correct and error if not
typedef float MM_f32;
typedef double MM_f64;

typedef MM_u8 MM_bool;
#define MM_true (MM_bool)(1)
#define MM_false (MM_bool)(0)

// NOTE: umm and imm are replacements for uint and int, used for general computation where the size
//       is irrelevant and for pointer width integers
typedef MM_u64 MM_umm;
typedef MM_i64 MM_imm;

typedef struct MM_String
{
    MM_u8* data;
    MM_u32 size;
} MM_String;

#define MM_STRING(str_lit) (MM_String){ .data = (MM_u8*)(str_lit), .size = sizeof(str_lit) - 1 }

#define MM_ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define MM_CONCAT_(a, b) a##b
#define MM_CONCAT(a, b) MM_CONCAT_(a, b)
#define MM_STATIC_ASSERT(EX)                                                            \
struct MM_CONCAT(MM_STATIC_ASSERT_, MM_CONCAT(__COUNTER__, MM_CONCAT(_, __LINE__))) \
{                                                                                   \
int static_assert_fails_on_negative_bit_width : (EX) ? 1 : -1;                  \
}

// NOTE: This is used to force API contracts (See MM_Token_FirstFromString for an example)
//       the ifndef is so that it can be overridden, which it probably shouldn't be
#ifndef MM_CONTRACT_ASSERT
#define MM_CONTRACT_ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0))
#endif

#if MM_DEBUG
#define MM_NOT_IMPLEMENTED *(volatile int*)0 = 0
#define MM_ILLEGAL_CODE_PATH *(volatile int*)0 = 0
#else
#define MM_NOT_IMPLEMENTED MM_STATIC_ASSERT(!"MM_NOT_IMPLEMENTED")
#define MM_ILLEGAL_CODE_PATH
#endif

#define MM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MM_MAX(a, b) ((a) < (b) ? (b) : (a))
#define MM_ROUND_UP(N, align) (((MM_umm)(N) + ((MM_umm)(align) - 1)) & ~((MM_umm)(align) - 1))
#define MM_ROUND_DOWN(N, align) ((MM_umm)(N) & ~((MM_umm)(align) - 1))
#define MM_IS_POW_2(N) ((((MM_umm)(N) - 1) & (MM_umm)(N)) == 0 && (N) > 0)

typedef MM_String MM_Identifier;
typedef MM_String MM_String_Literal;

MM_bool
MM_Identifier_IsBlank(MM_Identifier ident)
{
    return (ident.size == 0 || ident.size == 1 && *ident.data == '_');
}

#include <intrin.h>

#include "mm_string.h"
#include "mm_memory.h"
#include "mm_soft_int.h"
#include "mm_float.h"
#include "mm_tokens.h"
#include "mm_ast.h"
#include "mm_parser.h"

// NOTE: API
MM_Token MM_Token_FirstFromString(MM_String string, MM_u32 skip, MM_Text_Pos init_pos, MM_Text_Pos* start_pos, MM_Text_Pos* end_pos);

MM_bool MM_Parser_ParseString(MM_String string, MM_Text_Pos pos, MM_Arena* ast_arena, MM_Arena* string_arena, MM_AST** ast);

#endif