#ifndef MM_H
#define MM_H

#include "mm_base_types.h"
#include "mm_utility_macros.h"

/// Workspace API
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The workspace is intended to be a way for the compiler to keep state about the compilation and provide
//       certain guarantees (like "deterministic" compilation). The implementation of a workspace and its functions
//       are therefore hidden (i.e. not included in this header, but still available in the library implementation)
//       to communicate that they should not be tampered with (although, if you want, you could always use the
//       information in the library implementation to do whatever)

typedef void* MM_Workspace;

MM_API MM_Workspace MM_Workspace_Open ();
MM_API void         MM_Workspace_Close(MM_Workspace workspace);

// TODO: intended usage

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Memory
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline MM_umm MM_RoundUp    (MM_umm n, MM_umm alignment);
inline MM_umm MM_RoundDown  (MM_umm n, MM_umm alignment);
inline void*  MM_Align      (void* ptr, MM_u8 alignment);
inline MM_u8  MM_AlignOffset(void* ptr, MM_u8 alignment);
void          MM_Copy       (void* src, void* dst, MM_umm size);
void          MM_Move       (void* src, void* dst, MM_umm size);
void          MM_Zero       (void* ptr, MM_umm size);

// Macros:
// MM_ZeroStruct(S)   - zero the bytes of a struct S by pointer
// MM_ZeroArray(A, C) - zero the bytes of an array A with C elements of *A size

MM_TYPEDEF_FUNC(void*, MM_Reserve_Memory_Func, MM_umm size);
MM_TYPEDEF_FUNC(void,  MM_Commit_Memory_Func, void* commit_base, MM_umm size);
MM_TYPEDEF_FUNC(void,  MM_Free_Memory_Func, void* reserve_base);

struct MM_Arena;
typedef struct MM_Arena MM_Arena;
MM_Arena* MM_Arena_Init (MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func);
void*     MM_Arena_Push (MM_Arena* arena, MM_umm size, MM_u8 alignment);
void      MM_Arena_Clear(MM_Arena* arena);
void      MM_Arena_Free (MM_Arena* arena);

#include "mm_memory.h" // NOTE: definitions of MM_Arena and MM_DynArena, as well as implementation of the following

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Strings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_string.h" // NOTE: definitions of MM_String and MM_Interned_String, as well as implementation of the following
MM_String MM_String_WrapCString(char* cstring);
MM_bool   MM_String_Match      (MM_String s0, MM_String s1); // NOTE: Match, as in "do these strings match / are they equal"
MM_u64    MM_String_Hash       (MM_String string);

// Macros:
// MM_STRING(str) - initialize MM_String from a constant c string literal

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// AST
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_ast.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// F16
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_f16.h" // NOTE: implementation of the following
MM_f16 MM_F64_ToF16  (MM_f64 float64);
MM_f64 MM_F64_FromF16(MM_f16 float16);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Lexer
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mm_lexer.h" // NOTE: definitions of MM_Lexer and MM_Token, as well as implementation of the following:
MM_Lexer  MM_Lexer_Init           (MM_String string);
MM_umm    MM_Lexer_NextTokens     (MM_Lexer* lexer, MM_Token* buffer, MM_umm amount);
MM_Token  MM_Lexer_NextToken      (MM_Lexer* lexer);
MM_i128   MM_Lexer_ParseInt       (MM_Lexer* lexer, MM_Token token);
MM_f64    MM_Lexer_ParseFloat     (MM_Lexer* lexer, MM_Token token);
MM_u32    MM_Lexer_ParseChar      (MM_Lexer* lexer, MM_Token token);
MM_String MM_Lexer_ParseIdentifier(MM_Lexer* lexer, MM_Token token);
MM_String MM_Lexer_ParseString    (MM_Lexer* lexer, MM_Token token, MM_u8* buffer);

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