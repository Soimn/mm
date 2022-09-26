void
MM_Memzero(void* ptr, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i) ((MM_u8*)ptr)[i] = 0;
}

#define MM_ZeroStruct(ptr) MM_Memzero((ptr), sizeof(*(ptr)))

void
MM_Memcopy(void* dst, void* src, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i) ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
}

void
MM_Memmove(void* dst, void* src, MM_umm size)
{
    if ((MM_umm)dst > (MM_umm)src) for (MM_imm i = size - 1; i >= 0; --i) ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
    else                           MM_Memcopy(dst, src, size);
}