internal inline umm
RoundUp(umm n, umm alignment)
{
    return (n + (alignment - 1)) & ~alignment;
}

internal inline umm
RoundDown(umm n, umm alignment)
{
    return n & ~alignment;
}

internal inline void*
Align(void* ptr, u8 alignment)
{
    return (void*)RoundUp((umm)ptr, alignment);
}

internal inline u8
AlignOffset(void* ptr, u8 alignment)
{
    return (u8)((u8*)Align(ptr, alignment) - (u8*)ptr);
}

internal inline void
Zero(void* ptr, umm size)
{
    for (u8* bptr = ptr;
         bptr < (u8*)ptr + size;
         ++bptr)
    {
        *bptr = 0;
    }
}

#define ZeroStruct(S) Zero((S), sizeof(*(S)))

internal inline void
Copy(void* src, void* dst, umm size)
{
    for (umm i = 0; i < size; ++i)
    {
        ((u8*)dst)[i] = ((u8*)src)[i];
    }
}

internal inline void
Move(void* src, void* dst, umm size)
{
    u8* bsrc = src;
    u8* bdst = dst;
    
    if (bsrc <= bdst && bsrc + size > bdst)
    {
        for (u64 i = size - 1; i < size; --i)
        {
            ((u8*)dst)[i] = ((u8*)src)[i];
        }
    }
    else Copy(src, dst, size);
}

#define ARENA_RESERVATION_SIZE GB(1)
typedef struct Arena
{
    u8* base;
    u64 offset;
    u64 size;
} Arena;

typedef u64 Arena_Marker;

internal void*
Arena_PushSize(Arena* arena, umm size, u8 alignment)
{
    if (arena->offset + size + AlignOffset(arena->base + arena->offset, alignment) > arena->size)
    {
        if (arena->base == 0)
        {
            arena->base = System_ReserveMemory(ARENA_RESERVATION_SIZE);
        }
        
        if (arena->base == 0 || !System_CommitMemory(arena->base + arena->size, KB(64)))
        {
            //// ERROR: Out of Memory
            ASSERT(!"OUT OF MEMORY");
        }
    }
    
    u8* cursor      = arena->base + arena->offset;
    u8 align_offset = AlignOffset(cursor, alignment);
    
    void* result = cursor + align_offset;
    
    arena->offset += align_offset + size;
    arena->size   -= align_offset + size;
    
    return result;
}

internal inline void
Arena_Clear(Arena* arena)
{
    arena->offset = 0;
}

internal inline void
Arena_Free(Arena* arena)
{
    System_FreeMemory(arena->base, ARENA_RESERVATION_SIZE);
    ZeroStruct(arena);
}

internal inline Arena_Marker
Arena_BeginTemp(Arena* arena)
{
    return (Arena_Marker)arena->offset;
}

internal inline void
Arena_EndTemp(Arena* arena, Arena_Marker marker)
{
    ASSERT(arena->offset >= marker);
    arena->offset = marker;
}