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

#define internal static
#define global static