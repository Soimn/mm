inline void*
MM_Align(void* ptr, MM_umm alignment)
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

#define MM_ARENA__PAGE_SIZE (4*1024)
#define MM_ARENA__INITIAL_CURSOR_OFFSET sizeof(MM_Arena)
#define MM_ARENA__INITIAL_CURSOR(a) ((MM_u8*)(a) + MM_ARENA__INITIAL_CURSOR_OFFSET)

typedef struct MM_Arena
{
    MM_u8* cursor;
    MM_u64 commit_size;
    MM_umm reserve_size;
} MM_Arena;

MM_STATIC_ASSERT(MM_ARENA__INITIAL_CURSOR_OFFSET <= MM_ARENA__PAGE_SIZE);
MM_STATIC_ASSERT(MM_IS_POW_2(MM_ARENA__PAGE_SIZE));

MM_Arena*
MM_Arena_Create(MM_umm reserve_size)
{
    reserve_size = MM_ROUND_UP(reserve_size, MM_ARENA__PAGE_SIZE);
    
    void* memory = MM_System_ReserveMemory(reserve_size);
    MM_System_CommitMemory(memory, MM_ARENA__PAGE_SIZE);
    
    // TODO: Handle out of memory
    
    MM_Arena* arena = memory;
    *arena = (MM_Arena){
        .cursor       = (MM_u8*)memory + MM_ARENA__INITIAL_CURSOR_OFFSET,
        .commit_size  = MM_ARENA__PAGE_SIZE,
        .reserve_size = reserve_size,
    };
    
    return arena;
}

void
MM_Arena_Destroy(MM_Arena* arena)
{
    MM_System_ReleaseMemory(arena);
}

MM_umm
MM_Arena_CurrentSize(MM_Arena* arena)
{
    return arena->cursor - MM_ARENA__INITIAL_CURSOR(arena);
}

void*
MM_Arena_Push(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    MM_u8 align_offset = MM_AlignOffset(arena->cursor, alignment);
    
    if (MM_ARENA__INITIAL_CURSOR_OFFSET + MM_Arena_CurrentSize(arena) + align_offset + size > arena->commit_size)
    {
        MM_umm current_page_usage = ((MM_umm)arena->cursor - (MM_umm)arena) & (MM_ARENA__PAGE_SIZE - 1);
        MM_umm spill              = (current_page_usage + align_offset + size) - MM_ARENA__PAGE_SIZE;
        MM_umm to_commit  = MM_ROUND_UP(spill + MM_ARENA__PAGE_SIZE/2, MM_ARENA__PAGE_SIZE);
        // NOTE: ^ adding spill + half page size, so that an extra page is committed when the last is more than half full
        
        arena->commit_size += to_commit;
        MM_ASSERT(arena->commit_size <= arena->reserve_size); // TODO: Handle out of memory
        
        MM_System_CommitMemory(MM_Align(arena->cursor, MM_ARENA__PAGE_SIZE), to_commit);
    }
    
    void* result = arena->cursor + align_offset;
    
    arena->cursor += align_offset + size;
    
    return result;
}

void
MM_Arena_Pop(MM_Arena* arena, MM_umm size)
{
    arena->cursor -= MM_MIN(size, MM_Arena_CurrentSize(arena));
}

void
MM_Arena_Reset(MM_Arena* arena)
{
    arena->cursor = MM_ARENA__INITIAL_CURSOR(arena);
}