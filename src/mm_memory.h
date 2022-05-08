internal inline umm
RoundUp(umm n, umm alignment)
{
    return (n + (alignment - 1)) & ~(alignment - 1);
}

internal inline umm
RoundDown(umm n, umm alignment)
{
    return n & ~(alignment - 1);
}

internal inline void*
Align(void* ptr, u8 alignment)
{
    return (void*)RoundUp((umm)ptr, alignment);
}

internal inline u8
AlignOffset(void* ptr, u8 alignment)
{
    return (u8)(RoundUp((umm)ptr, alignment) - (umm)ptr);
}

internal void
Copy(void* src, void* dst, umm size)
{
    for (umm i = 0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}

internal void
Move(void* src, void* dst, umm size)
{
    u8* bsrc = (u8*)src;
    u8* bdst = (u8*)dst;
    
    if (bdst <= bsrc || bsrc + size < bdst) Copy(src, dst, size);
    else for (umm i = 0; i < size; ++i) ((u8*)dst)[size - i] = ((u8*)src)[size - i];
}

internal void
Zero(void* ptr, umm size)
{
    for (umm i = 0; i < size; ++i) ((u8*)ptr)[i] = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(*(S)))

#define ARENA_RESERVE_SIZE GB(10)

typedef struct Arena
{
    u64 offset;
    u64 space;
    u64 commit_size;
} Arena;

typedef u64 Arena_Marker;

internal Arena*
Arena_Init(u8 num_pages_per_commit)
{
    umm page_size    = System_PageSize();
    umm reserve_size = RoundUp(ARENA_RESERVE_SIZE, page_size);
    umm commit_size  = page_size * num_pages_per_commit;
    
    Arena* arena = System_ReserveMemory(reserve_size);
    System_CommitMemory(arena, commit_size);
    
    *arena = (Arena){
        .offset      = 0,
        .space       = commit_size,
        .commit_size = commit_size,
    };
    
    return arena;
}

internal void*
Arena_PushSize(Arena* arena, umm size, u8 alignment)
{
    u8* cursor   = (u8*)(arena + 1) + arena->offset;
    void* result = Align(cursor, alignment);
    
    umm advancement = ((u8*)result + size) - cursor;
    
    if (advancement > arena->space)
    {
        umm commit_size = (size <= arena->commit_size ? arena->commit_size : RoundUp(size, System_PageSize()));
        System_CommitMemory(cursor + arena->space, commit_size);
        arena->space += commit_size;
    }
    
    arena->offset += advancement;
    arena->space  -= advancement;
    
    return result;
}

internal void
Arena_Clear(Arena* arena)
{
    arena->space += arena->offset;
    arena->offset = 0;
}

internal void
Arena_Free(Arena* arena)
{
    System_FreeMemory(arena);
}

internal Arena_Marker
Arena_BeginTemp(Arena* arena)
{
    return arena->offset;
}

internal void
Arena_EndTemp(Arena* arena, Arena_Marker marker)
{
    ASSERT(marker < arena->offset);
    
    arena->space += arena->offset - marker;
    arena->offset = marker;
}

internal void
Arena_ReifyTemp(Arena* arena, Arena_Marker marker)
{
    ASSERT(marker < arena->offset);
}

internal void
Arena_ReifyPartialTemp(Arena* arena, Arena_Marker marker, umm size)
{
    ASSERT(marker < arena->offset && marker + size < arena->offset);
    
    umm end = marker + size;
    arena->space += end - arena->offset;
    arena->offset = end;
}