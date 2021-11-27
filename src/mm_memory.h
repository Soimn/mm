internal inline umm
RoundUpToAlignment(umm size, umm alignment)
{
    umm overshoot    = size + (alignment - 1);
    umm rounded_down = overshoot & ~(alignment - 1);
    
    return rounded_down;
}

internal inline umm
RoundDownToAlignment(umm size, umm alignment)
{
    umm rounded_down = size & ~(alignment - 1);
    
    return rounded_down;
}

internal inline void*
Align(void* ptr, u8 alignment)
{
    return (void*)RoundUpToAlignment((umm)ptr, alignment);
}

internal inline u8
AlignOffset(void* ptr, u8 alignment)
{
    return (u8)(RoundUpToAlignment((umm)ptr, alignment) - (umm)ptr);
}

internal inline void
Copy(void* src, void* dst, umm size)
{
    u8* bsrc = src;
    u8* bdst = dst;
    
    for (umm i = 0; i < size; ++i)
    {
        *bdst++ = *bsrc++;
    }
}

internal inline void
Move(void* src, void* dst, umm size)
{
    u8* bsrc = src;
    u8* bdst = dst;
    
    if (bdst < bsrc || bsrc + size < bdst) Copy(src, dst, size);
    else
    {
        bsrc += size - 1;
        bdst += size - 1;
        
        for (umm i = 0; i < size; ++i)
        {
            *bdst-- = *bsrc--;
        }
    }
}

internal inline void
Zero(void* ptr, umm size)
{
    u8* bptr = ptr;
    
    for (umm i = 0; i < size; ++i)
    {
        *bptr++ = 0;
    }
}

#define ZeroStruct(X) Zero((X), sizeof(*(X)))
#define ZeroArray(X, S) Zero((X), sizeof(*(X)) * (S))

internal inline bool
MemoryCompare(void* a, void* b, umm size)
{
    bool are_equal = true;
    
    u8* ba = a;
    u8* bb = b;
    
    for (umm i = 0; i < size; ++i)
    {
        if (*ba++ != *bb++)
        {
            are_equal = false;
            break;
        }
    }
    
    return are_equal;
}

#define StructCompare(s0, s1) MemoryCompare((s0), (s1), sizeof(*(s0)))
#define ArrayCompare(a0, a1, size) MemoryCompare((a0), (a1), sizeof(*(a0)) * (size))

typedef struct Memory_Arena_Marker
{
    u64 offset;
} Memory_Arena_Marker;

typedef struct Memory_Arena
{
    u64 base_address;
    u64 offset;
    u64 size;
    
    u64 high_water_mark;
} Memory_Arena;

internal inline void*
Arena_PushSize(Memory_Arena* arena, umm size, u8 alignment)
{
    u64 base  = arena->base_address + arena->offset;
    u8 offset = AlignOffset((void*)base, alignment);
    
    ASSERT(arena->offset + offset + size <= arena->size);
    
    arena->offset += offset + size;
    
#if IZ_INTERNAL || IZ_DEBUG
    arena->high_water_mark = MAX(arena->high_water_mark, arena->offset);
#endif
    
    return (void*)(base + offset);
}

internal inline void
Arena_Clear(Memory_Arena* arena)
{
    arena->offset = 0;
}

internal Memory_Arena_Marker
Arena_BeginTempMemory(Memory_Arena* arena)
{
    return (Memory_Arena_Marker){arena->offset};
}

internal void
Arena_EndTempMemory(Memory_Arena* arena, Memory_Arena_Marker marker)
{
    arena->offset = marker.offset;
}