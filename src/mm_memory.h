void
MM_Copy(void* src, void* dst, MM_umm size)
{
    MM_u8* bsrc = (MM_u8*)src;
    MM_u8* bdst = (MM_u8*)dst;
    for (MM_umm i = 0; i < size; ++i) bdst[i] = bsrc[i];
}