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

internal inline bool
Memcmp(void* a, void* b, umm size)
{
    bool result = true;
    
    for (umm i = 0; i < size; ++i)
    {
        if (((u8*)a)[i] != ((u8*)b)[i])
        {
            result = false;
            break;
        }
    }
    
    return result;
}

#define ARENA_RESERVATION_SIZE GB(1)
#define ARENA_PAGE_SIZE KB(16)
typedef struct Arena
{
    u8* base;
    u64 offset;
    u64 size;
} Arena;

typedef u64 Arena_Marker;

internal Arena*
Arena_Init()
{
    Arena* arena = 0;
    
    void* memory = System_ReserveMemory(ARENA_RESERVATION_SIZE);
    
    if (memory == 0 || !System_CommitMemory(memory, ARENA_PAGE_SIZE))
    {
        //// ERROR: Out of Memory
        ASSERT(!"OUT OF MEMORY");
    }
    else
    {
        arena = memory;
        *arena = (Arena){
            arena->base = (u8*)(arena + 1),
            arena->offset = 0,
            arena->size   = ARENA_PAGE_SIZE,
        };
    }
    
    return arena;
}

internal void*
Arena_PushSize(Arena* arena, umm size, u8 alignment)
{
    if (arena->offset + size + AlignOffset(arena->base + arena->offset, alignment) > arena->size)
    {
        if (arena->base == 0 || !System_CommitMemory(arena->base + arena->size, ARENA_PAGE_SIZE))
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

internal inline void
Arena_ReifyTemp(Arena* arena, Arena_Marker marker)
{
    ASSERT(arena->offset >= marker);
}