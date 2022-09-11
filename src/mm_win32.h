#define UNICODE
#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near
#undef MM_MIN
#undef MM_MAX

#include <intrin.h>
#include <immintrin.h>

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

#include "mm_common.h"

#define MM_SYSTEM_PAGE_SIZE MM_KB(4)

void*
MM_System_ReserveMemory(MM_umm size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

void
MM_System_CommitMemory(void* base, MM_umm size)
{
    VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
}

void
MM_System_FreeMemory(void* base)
{
    VirtualFree(base, 0, MEM_RELEASE);
}

int
mainCRTStartup()
{
    void MM_Main();
    MM_Main();
    return 0;
}