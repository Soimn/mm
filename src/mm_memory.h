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

void*
MM_Align(void* ptr, MM_u8 alignment)
{
    return (void*)MM_ROUND_UP(ptr, alignment);
}

MM_u8
MM_AlignOffset(void* ptr, MM_u8 alignment)
{
    return (MM_u8)(MM_ROUND_UP(ptr, alignment) - (MM_umm)ptr);
}

#define MM_ALIGNOF(T) ((MM_u8)(MM_umm)&((struct{ char c; T t; }*)0)->t)

#define MM_ARENA_PAGE_SIZE (16*1024)

typedef struct MM_Arena
{
    MM_u64 cursor;
} MM_Arena;

MM_Arena*
MM_Arena_Create()
{
    MM_Arena* arena = MM_System_ReserveMemory((MM_umm)32 * 1024*1024*1024);
    MM_System_CommitMemory(arena, MM_ARENA_PAGE_SIZE);
    
    *arena = (MM_Arena){
        .cursor = sizeof(MM_Arena),
    };
    
    return arena;
}

void*
MM_Arena_Push(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    MM_ASSERT(size <= MM_U32_MAX);
    MM_ASSERT(MM_IS_POW_2(alignment));
    
    MM_u8 offset = MM_AlignOffset(arena + arena->cursor, alignment);
    
    if ((arena->cursor % MM_ARENA_PAGE_SIZE) + size + offset > MM_ARENA_PAGE_SIZE)
    {
        MM_System_CommitMemory(arena + MM_ROUND_UP(arena->cursor, MM_ARENA_PAGE_SIZE), MM_ARENA_PAGE_SIZE);
    }
    
    void* result = (MM_u8*)arena + arena->cursor + offset;
    
    arena->cursor += offset + size;
    
    return result;
}

void
MM_Arena_Pop(MM_Arena* arena, MM_umm size)
{
    MM_ASSERT(arena->cursor - sizeof(MM_Arena) >= size);
    
    arena->cursor -= size;
}

void
MM_Arena_Destroy(MM_Arena* arena)
{
    MM_System_FreeMemory(arena);
}