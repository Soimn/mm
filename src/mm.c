#include "mm.h"

#ifdef _WIN32

void* memset(void* ptr, int value, unsigned __int64 size);
void* memcpy(void* rdst, const void* rsrc, unsigned __int64 count);

void*
memset(void* ptr, int value, unsigned __int64 size)
{
    unsigned __int8* bptr = ptr;
    unsigned __int8 val   = (unsigned __int8)value;
    
    for (unsigned __int64 i = 0; i < size; ++i)
    {
        *bptr++ = val;
    }
    
    return ptr;
}

void*
memcpy(void* rdst, const void* rsrc, unsigned __int64 count)
{
    unsigned __int8* dst = (unsigned __int8*)rdst;
    const unsigned __int8* src = (const unsigned __int8*)rsrc;
    while (count--)
    {
        *dst++ = *src++;
    }
    return dst;
}

int _fltused;

int __stdcall _DllMainCRTStartup(void* hinstDLL, unsigned int fdwReason, void* lpReserved)
{
    return 1;
}
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

#define MM_Internal static

typedef void* MM_Type_ID;

#include "mm_memory.c"
#include "mm_string.c"
#include "mm_f16.c"
#include "mm_lexer.c"
#include "mm_parser.c"
#include "mm_symbols.c"
#include "mm_types.c"
#include "mm_checker.c"

typedef struct MM_Workspace
{
    union
    {
        MM_Arena* arena_bank[3];
        struct
        {
            MM_Arena* ws_arena;  // NOTE: for storing the MM_Workspace and other misc structures
            MM_Arena* ast_arena; // NOTE: for storing MM_AST nodes
            MM_Arena* str_arena; // NOTE: for storing string literals
        };
    };
    
    
} MM_Workspace;

MM_API MM_Workspace*
MM_Workspace_Open(MM_Workspace_Settings settings)
{
    MM_Workspace* workspace = 0;
    
    MM_Arena* ws_arena = MM_Arena_Init(settings.reserve_func, settings.commit_func, settings.free_func);
    
    if (ws_arena != 0)
    {
        workspace = MM_Arena_Push(ws_arena, sizeof(MM_Workspace), MM_ALIGNOF(MM_Workspace));
        MM_ZeroStruct(workspace);
        
        workspace->ws_arena = ws_arena;
        for (MM_umm i = 1; i < MM_ARRAY_SIZE(workspace->arena_bank); ++i)
        {
            // TODO: What to do if the init fails? What to do when a later commit fails?
            workspace->arena_bank[i] = MM_Arena_Init(settings.reserve_func, settings.commit_func, settings.free_func);
        }
    }
    
    return workspace;
}

MM_API void
MM_Workspace_Close(MM_Workspace* workspace)
{
    for (MM_umm i = 1; i < MM_ARRAY_SIZE(workspace->arena_bank); ++i)
    {
        MM_Arena_Free(workspace->arena_bank[i]);
    }
    
    MM_Arena_Free(workspace->ws_arena);
}

MM_API void
MM_Workspace_AddDecl(MM_Workspace* workspace, struct MM_Declaration* decl)
{
    // TODO: Copy and gen symbols
    MM_NOT_IMPLEMENTED;
}