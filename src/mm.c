#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

#ifdef _WIN32
#include "mm_win32.h"
#else
#error "Platform not supported"
#endif

void* MM_System_ReserveMemory(MM_umm size);
void  MM_System_CommitMemory (void* base, MM_umm size);
void  MM_System_FreeMemory   (void* base);

#include "mm_memory.h"
#include "mm_int.h"
#include "mm_float.h"
#include "mm_string.h"

typedef MM_u32 MM_File_ID;

typedef struct MM_Text_Pos
{
    MM_File_ID file;
    MM_u32 offset;
} MM_Text_Pos;

typedef struct MM_Text
{
    union
    {
        struct MM_Text_Pos;
        MM_Text_Pos pos;
    };
    MM_u32 size;
} MM_Text;

typedef struct MM_Identifier
{
} MM_Identifier;

typedef struct MM_String_Literal
{
} MM_String_Literal;


typedef struct MM_Codepoint
{
    MM_u32 value;
} MM_Codepoint;

#include "mm_lexer.h"

void
MM_Main()
{
}