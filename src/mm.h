#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifndef MM_API
#define MM_API
#endif

#ifndef _WIN64
#error 32 bit mode is not supported yet
#endif

typedef signed __int8  MM_i8;
typedef signed __int16 MM_i16;
typedef signed __int32 MM_i32;
typedef signed __int64 MM_i64;

typedef unsigned __int8  MM_u8;
typedef unsigned __int16 MM_u16;
typedef unsigned __int32 MM_u32;
typedef unsigned __int64 MM_u64;

typedef MM_u64 MM_umm;
typedef MM_i64 MM_imm;

typedef float  MM_f32;
typedef double MM_f64;

typedef MM_u8 MM_bool;

typedef struct MM_String
{
    MM_u8* data;
    MM_u64 size;
} MM_String;

#define MM_STRING(str) (MM_String){ .data = (MM_u8*)(str), .size = sizeof(str) - 1 }

typedef struct MM_String_Table
{
    // somewhere to store the strings
    // somewhere to store the linked entries
    struct MM__String_Table_Entry* entries;
    MM_u32 entry_count;
} MM_String_Table;

typedef MM_u64 MM_Interned_String;

MM_Interned_String MM_InternedString_FromString(MM_String_Table* string_table, MM_String string);
MM_String MM_InternedString_ToString(MM_String_Table* string_table, MM_Interned_String istring);

typedef struct MM_Big_Int
{
    MM_bool _stub;
} MM_Big_Int;

MM_Big_Int MM_BigInt_FromU64(MM_u64 u64);
MM_Big_Int MM_BigInt_Add(MM_Big_Int a, MM_Big_Int b);
MM_Big_Int MM_BigInt_Sub(MM_Big_Int a, MM_Big_Int b);
MM_Big_Int MM_BigInt_Mul(MM_Big_Int a, MM_Big_Int b);
MM_Big_Int MM_BigInt_Div(MM_Big_Int a, MM_Big_Int b);
MM_Big_Int MM_BigInt_Rem(MM_Big_Int a, MM_Big_Int b);
MM_Big_Int MM_BigInt_Pow10(MM_Big_Int exponent);

typedef struct MM_Big_Float
{
    MM_bool _stub;
} MM_Big_Float;

MM_Big_Float MM_BigFloat_FromParts(MM_Big_Int integer, MM_Big_Int fraction, MM_Big_Int exponent);

// NOTE: The token values are arranged in such a way that it allows mapping to expression kinds with precedence baked in.
//       This is a kind of unnecessarily clever trick to cache information gathered by the lexer which could not be
//       done by doing regular ASCII mapping. It is most likely an insignificant optimization, however it does not cost
//       much complexity to do, so why not.
// IMPORTANT NOTE: The main problem with this approach is that it is extremely fragile, extreme care must therefore
//                 be taken moving, renaming or otherwise changing any of the values in MM_TOKEN_KIND and MM_EXPR_KIND
enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    
    MM_Token_TripleMinus,                                   // ---
    MM_Token_Arrow,                                         // ->
    MM_Token_CloseParen,                                    // )
    MM_Token_CloseBracket,                                  // ]
    MM_Token_OpenBrace,                                     // {
    MM_Token_CloseBrace,                                    // }
    MM_Token_Comma,                                         // ,
    MM_Token_Colon,                                         // :
    MM_Token_Semicolon,                                     // ;
    MM_Token_Underscore,                                    // _
    MM_Token_QuestionMark,                                  // ?
    MM_Token_Identifier,
    MM_Token_String,
    MM_Token_Character,
    MM_Token_Int,
    MM_Token_Float,
    
    
    MM_Token_FirstAssignment,
    MM_Token_Equals = MM_Token_FirstAssignment,             // =
    
    MM_Token_FirstMulLevelAssignment = 5*16,
    MM_Token_StarEquals = MM_Token_FirstMulLevelAssignment, // *=
    MM_Token_SlashEquals,                                   // /=
    MM_Token_RemEquals,                                     // %=
    MM_Token_AndEquals,                                     // &=
    MM_Token_ArithmeticRightShiftEquals,                    // >>>=
    MM_Token_RightShiftEquals,                              // >>=
    MM_Token_SplatLeftShiftEquals,                          // <<<=
    MM_Token_LeftShiftEquals,                               // <<=
    MM_Token_LastMulLevelAssignment = 6*16 - 1,
    
    MM_Token_FirstAddLevelAssignment = 6*16,
    MM_Token_PlusEquals = MM_Token_FirstAddLevelAssignment, // +=
    MM_Token_MinusEquals,                                   // -=
    MM_Token_OrEquals,                                      // |=
    MM_Token_HatEquals,                                     // ^=
    MM_Token_LastAddLevelAssignment = 7*16 - 1,
    
    MM_Token_AndAndEquals = 7*16,                           // &&=
    
    MM_Token_OrOrEquals = 8*16,                             // ||=
    
    MM_Token_LastAssignment = 9*16 - 1,
    
    
    MM_Token_FirstPostfixLevel = 9*16,
    MM_Token_OpenBracket,                                   // [
    MM_Token_OpenParen,                                     // (
    MM_Token_Period,                                        // .
    MM_Token_OpenPeriodBrace,                               // .{
    MM_Token_OpenPeriodBracket,                             // .[
    MM_Token_OpenPeriodParen,                               // .(
    MM_Token_LastPostfixLevel = 10*16 - 1,
    
    MM_Token_Not = 10*16,                                   // !
    MM_Token_Complement,                                    // ~
    
    MM_Token_FirstMulLevel = 11*16,
    MM_Token_Star = MM_Token_FirstMulLevel,                 // *
    MM_Token_And,                                           // &
    MM_Token_Slash,                                         // /
    MM_Token_Rem,                                           // %
    MM_Token_ArithmeticRightShift,                          // >>>
    MM_Token_RightShift,                                    // >>
    MM_Token_SplatLeftShift,                                // <<<
    MM_Token_LeftShift,                                     // <<
    MM_Token_LastMulLevel = 12*16 - 1,
    
    MM_Token_FirstAddLevel = 12*16,
    MM_Token_Plus = MM_Token_FirstAddLevel,                 // +
    MM_Token_Minus,                                         // -
    MM_Token_Or,                                            // |
    MM_Token_Hat,                                           // ^
    MM_Token_LastAddLevel = 13*16 - 1,
    
    MM_Token_FirstComparative = 13*16,
    MM_Token_EqualEquals = MM_Token_FirstComparative,       // ==
    MM_Token_NotEquals,                                     // !=
    MM_Token_Less,                                          // <
    MM_Token_Greater,                                       // >
    MM_Token_LessEquals,                                    // <=
    MM_Token_GreaterEquals,                                 // >=
    MM_Token_LastComparative = 14*16 - 1,
    
    MM_Token_AndAnd = 14*16,                                // &&
    
    MM_Token_OrOr = 15*16,                                  // ||
};

typedef struct MM_Token
{
    MM_u32 offset;
    MM_u32 size;
    MM_u32 line;
    MM_u32 column;
    MM_u8 kind;
    
    union
    {
        MM_Interned_String identifier;
        MM_String string;
        MM_Big_Int integer;
        MM_Big_Float floating;
        MM_u8 character;
    };
} MM_Token;

enum MM_EXPR_KIND
{
    MM_Expr_Invalid = 0,
    
    MM_Expr_Identifier,
    MM_Expr_String,
    MM_Expr_Char,
    MM_Expr_Int,
    MM_Expr_Float,
    MM_Expr_Boolean,
    MM_Expr_Proc,
    MM_Expr_Struct,
    MM_Expr_Union,
    MM_Expr_Enum,
    MM_Expr_Compound,
    MM_Expr_Conditional,
    
    MM_Expr_FirstTypeLevel = 8*16,
    MM_Expr_PointerType = MM_Expr_FirstTypeLevel,
    MM_Expr_SliceType,
    MM_Expr_ArrayType,
    MM_Expr_LastTypeLevel = 9*16 - 1,
    
    MM_Expr_FirstPostfixLevel = 9*16,
    MM_Expr_Subscript = MM_Expr_FirstPostfixLevel,
    MM_Expr_Slice,
    MM_Expr_Call,
    MM_Expr_ElementOf,
    MM_Expr_StructLiteral,
    MM_Expr_ArrayLiteral,
    MM_Expr_Cast,
    MM_Expr_LastPostfixLevel = 10*16 - 1,
    
    MM_Expr_FirstPrefixLevel = 10*16,
    MM_Expr_Not = MM_Expr_FirstPrefixLevel,
    MM_Expr_Complement,
    MM_Expr_Dereference,
    MM_Expr_Reference,
    // NOTE: empty, unary + is parsed but no node is emitted
    MM_Expr_Negation = MM_Expr_Reference + 1,
    MM_Expr_LastPrefixLevel = 11*16 - 1,
    
    MM_Expr_FirstMulLevel = 11*16,
    MM_Expr_Mul = MM_Expr_FirstMulLevel,
    MM_Expr_BitAnd,
    MM_Expr_Div,
    MM_Expr_Rem,
    MM_Expr_BitSar,
    MM_Expr_BitShr,
    MM_Expr_BitSplatShl,
    MM_Expr_BitShl,
    MM_Expr_LastMulLevel = 12*16 - 1,
    
    MM_Expr_FirstAddLevel = 12*16,
    MM_Expr_Add = MM_Expr_FirstAddLevel,
    MM_Expr_Sub,
    MM_Expr_BitOr,
    MM_Expr_BitXor,
    MM_Expr_LastAddLevel = 13*16 - 1,
    
    MM_Expr_FirstComparative = 13*16,
    MM_Expr_IsEqual = MM_Expr_FirstComparative,
    MM_Expr_IsNotEqual,
    MM_Expr_IsLess,
    MM_Expr_IsGreater,
    MM_Expr_IsLessEqual,
    MM_Expr_IsGreaterEqual,
    MM_Expr_LastComparative = 14*16 - 1,
    
    MM_Expr_And = 14*16,
    
    MM_Expr_Or = 15*16,
};

enum MM_KEYWORD_KIND
{
    MM_Keyword_Proc,
    MM_Keyword_Struct,
    MM_Keyword_Union,
    MM_Keyword_Enum,
    MM_Keyword_True,
    MM_Keyword_False,
    MM_Keyword_As,
    MM_Keyword_If,
    MM_Keyword_Else,
    MM_Keyword_When,
    MM_Keyword_While,
    MM_Keyword_Break,
    MM_Keyword_Continue,
    MM_Keyword_Using,
    MM_Keyword_Defer,
    MM_Keyword_Return,
    
    MM_KEYWORD_KIND_COUNT,
};

enum MM_ERROR_CODE
{
    MM_Error_Success = 0,
    
    // TODO: ...
    
    MM_ERROR_CODE_COUNT,
};

typedef struct MM_Error
{
    struct MM_Error* next;
    MM_u32 error_code;
    // TODO: error info
} MM_Error;

//Reserve 40*string_size
//Commit as needed
MM_API MM_bool MM_LexString(MM_String string, MM_String_Table* string_table, MM_Array(MM_Token)* tokens, MM_Error* error);

#define true 1
#define false 0

#include "mm_common.h"
#include "mm_lexer.h"

#undef true
#undef false