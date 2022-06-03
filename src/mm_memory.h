inline MM_umm
MM_RoundUp(MM_umm n, MM_umm alignment)
{
    return (n + (alignment - 1)) & ~(alignment - 1);
}

inline MM_umm
MM_RoundDown(MM_umm n, MM_umm alignment)
{
    return n & ~(alignment - 1);
}

inline void*
MM_Align(void* ptr, MM_u8 alignment)
{
    return (void*)MM_RoundUp((MM_umm)ptr, alignment);
}

inline MM_u8
MM_AlignOffset(void* ptr, MM_u8 alignment)
{
    return (MM_u8)(MM_RoundUp((MM_umm)ptr, alignment) - (MM_umm)ptr);
}

void
MM_Copy(void* src, void* dst, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i) ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
}

void
MM_Move(void* src, void* dst, MM_umm size)
{
    MM_u8* bsrc = (MM_u8*)src;
    MM_u8* bdst = (MM_u8*)dst;
    
    if (bdst <= bsrc || bsrc + size < bdst) MM_Copy(src, dst, size);
    else for (MM_umm i = 0; i < size; ++i) ((MM_u8*)dst)[size - i] = ((MM_u8*)src)[size - i];
}

void
MM_Zero(void* ptr, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i) ((MM_u8*)ptr)[i] = 0;
}

#define MM_ZeroStruct(S) MM_Zero((S), sizeof(*(S)))
#define MM_ZeroArray(A, C) MM_Zero((A), sizeof(*(A)) * (C))

typedef struct MM_Arena_Block
{
    struct MM_Arena_Block* next;
    MM_u32 offset;
    MM_u32 space;
} MM_Arena_Block;

#define MM_ARENA_BLOCK_RESERVE_SIZE MM_GB(4)
typedef struct MM_Arena
{
    MM_Reserve_Memory_Func reserve_func;
    MM_Commit_Memory_Func commit_func;
    MM_Free_Memory_Func free_func;
    struct MM_Arena_Block* current_block;
    
    union
    {
        struct MM_Arena_Block;
        MM_Arena_Block block;
    };
} MM_Arena;

MM_Arena*
MM_Arena_Init(MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func)
{
    MM_Arena* arena = reserve_func(MM_ARENA_BLOCK_RESERVE_SIZE);
    
    if (arena != 0)
    {
        *arena = (MM_Arena){
            .reserve_func  = reserve_func,
            .commit_func   = commit_func,
            .free_func     = free_func,
            .current_block = &arena->block,
        };
        
        arena->current_block = (MM_Arena_Block){
            .next   = 0,
            .offset = sizeof(MM_Arena_Block),                         // NOTE: offset is relative to the block
            .space  = MM_ARENA_BLOCK_RESERVE_SIZE - sizeof(MM_Arena), // NOTE: space is relative to the reserve base
        };
    }
    
    return arena;
}

void*
MM_Arena_Push(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    MM_Arena_Block* block = arena->current_block;
    
    if (block->offset)
}

void
MM_Arena_Clear(MM_Arena* arena)
{
    arena->current_block = &arena->block;
    arena->current_block->offset = sizeof(MM_Arena_Block);
    arena->current_block->space  = MM_ARENA_BLOCK_RESERVE_SIZE - sizeof(MM_Arena);
}

void
MM_Arena_Free(MM_Arena* arena)
{
    arena->free_func(arena);
}