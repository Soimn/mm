inline void*
MM_Align(void* ptr, MM_u8 alignment)
{
    return (void*)MM_ROUND_UP((MM_umm)ptr, (MM_umm)alignment);
}

inline MM_u8
MM_AlignOffset(void* ptr, MM_u8 alignment)
{
    return (MM_u8)((MM_u8*)MM_Align(ptr, alignment) - (MM_u8*)ptr);
}

inline void
MM_Memcpy(void* dst, void* src, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i)
    {
        ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
    }
}

inline void
MM_Memmove(void* dst, void* src, MM_umm size)
{
    if ((MM_u8*)dst > (MM_u8*)src) MM_Memcpy(dst, src, size);
    else
    {
        for (MM_imm i = size - 1; i >= 0; --i)
        {
            ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
        }
    }
}

inline void
MM_Memset(void* ptr, MM_u8 val, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i)
    {
        ((MM_u8*)ptr)[i] = val;
    }
}

inline void
MM_Memzero(void* ptr, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i)
    {
        ((MM_u8*)ptr)[i] = 0;
    }
}

#define MM_ZeroStruct(ptr) MM_Memzero((ptr), sizeof(*(ptr)))
#define MM_ZeroArray(ptr, size) MM_Memzero((ptr), (size)*sizeof(*(ptr)))