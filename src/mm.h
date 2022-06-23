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

typedef struct MM_Arena MM_Arena;
typedef void* MM_Arena_Marker;
MM_API MM_Arena*       MM_Arena_Init     (MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func);
MM_API void*           MM_Arena_Push     (MM_Arena* arena, MM_umm size, MM_u8 alignment);
MM_API void            MM_Arena_Pop      (MM_Arena* arena, MM_umm amount);
MM_API MM_Arena_Marker MM_Arena_GetMarker(MM_Arena* arena);
MM_API void            MM_Arena_PopBack  (MM_Arena* arena, MM_Arena_Marker marker);
MM_API void            MM_Arena_Clear    (MM_Arena* arena);
MM_API void            MM_Arena_Free     (MM_Arena* arena);
MM_API void            MM_Arena_FreeEveryBlockAfterCurrent(MM_Arena* arena);

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
MM_API MM_Lexer       MM_Lexer_Init                    (MM_String string);
MM_API MM_Token       MM_Lexer_GetToken                (MM_Lexer* lexer);
MM_API MM_Token_Array MM_Lexer_NextTokens              (MM_Lexer* lexer, MM_Token* buffer, MM_umm amount);
MM_API MM_Token       MM_Lexer_NextToken               (MM_Lexer* lexer);
MM_API MM_String      MM_Lexer_TokenToString           (MM_Lexer* lexer, MM_Token token);
MM_API MM_i128        MM_Lexer_ParseInt                (MM_Lexer* lexer, MM_Token token);
MM_API MM_f64         MM_Lexer_ParseFloat              (MM_Lexer* lexer, MM_Token token);
MM_API MM_u32         MM_Lexer_ParseCodepoint          (MM_Lexer* lexer, MM_Token token);
MM_API MM_String      MM_Lexer_ParseString             (MM_Lexer* lexer, MM_Token token, MM_u8* buffer); // NOTE: buffer should be at least token.size long
MM_API MM_i128        MM_Lexer_ParseIntFromString      (MM_String string);
MM_API MM_f64         MM_Lexer_ParseFloatFromString    (MM_String string);
MM_API MM_u32         MM_Lexer_ParseCodepointFromString(MM_String string);
MM_API MM_String      MM_Lexer_ParseStringFromString   (MM_String string, MM_u8* buffer); // NOTE: buffer should be at least token.size long
MM_API MM_Token_Array MM_Lexer_WrapTokens              (MM_Lexer* lexer, MM_Token* tokens, MM_umm count);

// NOTE: The lexer implementation is catered to the imagined needs of text editors.
//       The intended usage is for the lexer to be initialized by MM_Lexer_Init,
//       and then used to produce one or several tokens with MM_Lexer_NextToken
//       and MM_Lexer_NextTokens. The lexer keeps track of the last encountered
//       token, and this can be queried with MM_Lexer_GetToken. If the kind and
//       placement is not enough information, the additional MM_LexerParse*
//       functions can be used to extract that information.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Parser
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct MM_Parser_Error
{
    // TODO
} MM_Parser_Error;

// NOTE: The return value is 0 when there was an error, otherwise it is the root node of the resulting AST
// NOTE: The error report parameter is optional. If it is not 0 and an error has occured, the error message and other
//       related information is written to the structure pointed to by "report".
MM_API MM_Expression*                 MM_Parser_ParseExpressionWithLexer                (MM_Lexer*, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementWithLexer                 (MM_Lexer*, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementListWithLexer             (MM_Lexer*, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_AssignmentOrExpression*     MM_Parser_ParseAssignmentOrExpressionWithLexer    (MM_Lexer*, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_DeclAssignmentOrExpression* MM_Parser_ParseDeclAssignmentOrExpressionWithLexer(MM_Lexer*, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);

MM_API MM_Expression*                 MM_Parser_ParseExpressionFromTokens                (MM_Token_Array, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementFromTokens                 (MM_Token_Array, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementListFromTokens             (MM_Token_Array, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_AssignmentOrExpression*     MM_Parser_ParseAssignmentOrExpressionFromTokens    (MM_Token_Array, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_DeclAssignmentOrExpression* MM_Parser_ParseDeclAssignmentOrExpressionFromTokens(MM_Token_Array, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);

MM_API MM_Expression*                 MM_Parser_ParseExpressionFromString                (MM_String, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementFromString                 (MM_String, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_Statement*                  MM_Parser_ParseStatementListFromString             (MM_String, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_AssignmentOrExpression*     MM_Parser_ParseAssignmentOrExpressionFromString    (MM_String, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
MM_API MM_DeclAssignmentOrExpression* MM_Parser_ParseDeclAssignmentOrExpressionFromString(MM_String, MM_Arena* ast, MM_Arena* strings, MM_Parser_Error* report);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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