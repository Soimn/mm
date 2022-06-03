#ifndef MM_H
#define MM_H

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifdef _WIN32
#ifdef MM_LIB
#define MM_API __declspec(dllexport)
#else
#define MM_API
#endif

#else
#error "platform not supported"
#endif

/// Basic Types
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////c
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

#define MM_TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////c

/// Workspace API
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////c
// NOTE: The workspace is intended to be a way for the compiler to keep state about the compilation and provide
//       certain guarantees (like "deterministic" compilation).

struct MM_Workspace;
typedef struct MM_Workspace MM_Workspace;
MM_API MM_Workspace* MM_Workspace_Open ();
MM_API void          MM_Workspace_Close(MM_Workspace* workspace);

// TODO: intended usage
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Memory
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MM_TYPEDEF_FUNC(void*, MM_Reserve_Memory_Func, MM_umm size);
MM_TYPEDEF_FUNC(void,  MM_Commit_Memory_Func, void* commit_base, MM_umm size);
MM_TYPEDEF_FUNC(void,  MM_Free_Memory_Func, void* reserve_base);

struct MM_Arena;
typedef struct MM_Arena MM_Arena;
MM_API MM_Arena* MM_Arena_Init (MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func);
MM_API void*     MM_Arena_Push (MM_Arena* arena, MM_umm size, MM_u8 alignment);
MM_API void      MM_Arena_Clear(MM_Arena* arena);
MM_API void      MM_Arena_Free (MM_Arena* arena);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Strings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct MM_String
{
    MM_u8* data;
    MM_u32 size;
} MM_String;

#define MM_STRING(str) (String){ .data = (MM_u8*)(str), .size = sizeof(str) - 1 }

MM_API MM_String MM_String_WrapCString(char* cstring);
MM_API MM_bool   MM_String_Match      (MM_String s0, MM_String s1); // NOTE: Match, as in "do these strings match / are they equal"
MM_API MM_u64    MM_String_Hash       (MM_String string);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// AST
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_ast.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// F16
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MM_API MM_f16 MM_F64_ToF16  (MM_f64 float64);
MM_API MM_f64 MM_F64_FromF16(MM_f16 float16);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Lexer
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_lexer.h" // NOTE: definitions of MM_Lexer and MM_Token, as well as implementation of the following:
MM_API MM_Lexer  MM_Lexer_Init           (MM_String string);
MM_API MM_umm    MM_Lexer_NextTokens     (MM_Lexer* lexer, MM_Token* buffer, MM_umm amount);
MM_API MM_Token  MM_Lexer_NextToken      (MM_Lexer* lexer);
MM_API MM_i128   MM_Lexer_ParseInt       (MM_Lexer* lexer, MM_Token token);
MM_API MM_f64    MM_Lexer_ParseFloat     (MM_Lexer* lexer, MM_Token token);
MM_API MM_u32    MM_Lexer_ParseChar      (MM_Lexer* lexer, MM_Token token);
MM_API MM_String MM_Lexer_ParseIdentifier(MM_Lexer* lexer, MM_Token token);
MM_API MM_String MM_Lexer_ParseString    (MM_Lexer* lexer, MM_Token token, MM_u8* buffer);

// NOTE: The lexer implementation is catered to the imagined needs of text editors.
//       The intended usage is for the lexer to be initialized by MM_Lexer_Init,
//       and then used to produce one or several tokens with MM_Lexer_NextToken
//       and MM_Lexer_NextTokens. If the kind and placement is not enough
//       information, the additional MM_LexerParse* functions can be used to
//       extract that information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






// ParseString()
// ParseTokens()


// TODO: intern string api

// TODO: load and store full state for debug info and branching (debug needs AST <-> instruction mapping information as well)
/*
MM_Check_Status MM_Workspace_Recheck(MM_Workspace workspace);


while (true)
{
    remove and update old declarations;
    insert new declarations;
    MM_Check_Status check_status = MM_Workspace_Recheck(workspace);
    
    if (check_status.code == Check_Completed)
    {
        report newc
    }
}*/

#endif