MM_Internal inline MM_umm
MM_RoundUp(MM_umm n, MM_umm alignment)
{
    return (n + (alignment - 1)) & ~(alignment - 1);
}

#define MM_ROUND_UP(n, alignment) (((n) + ((alignment) - 1)) & ~((alignment) - 1))

MM_Internal inline MM_umm
MM_RoundDown(MM_umm n, MM_umm alignment)
{
    return n & ~(alignment - 1);
}

#define MM_ROUND_DOWN(n, alignment) ((n) & ~((alignment) - 1))

MM_Internal inline void*
MM_Align(void* ptr, MM_u8 alignment)
{
    return (void*)MM_RoundUp((MM_umm)ptr, alignment);
}

MM_Internal inline MM_u8
MM_AlignOffset(void* ptr, MM_u8 alignment)
{
    return (MM_u8)(MM_RoundUp((MM_umm)ptr, alignment) - (MM_umm)ptr);
}

MM_Internal void
MM_Copy(void* src, void* dst, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i) ((MM_u8*)dst)[i] = ((MM_u8*)src)[i];
}

MM_Internal void
MM_Move(void* src, void* dst, MM_umm size)
{
    MM_u8* bsrc = (MM_u8*)src;
    MM_u8* bdst = (MM_u8*)dst;
    
    if (bdst <= bsrc || bsrc + size < bdst) MM_Copy(src, dst, size);
    else for (MM_umm i = 0; i < size; ++i) ((MM_u8*)dst)[size - i] = ((MM_u8*)src)[size - i];
}

MM_Internal void
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

#define MM_ARENA_PAGE_SIZE MM_KB(4)
#define MM_ARENA_BLOCK_RESERVE_SIZE MM_ROUND_UP(MM_GB(4), MM_ARENA_PAGE_SIZE)
typedef struct MM_Arena
{
    MM_Reserve_Memory_Func reserve_func;
    MM_Commit_Memory_Func commit_func;
    MM_Free_Memory_Func free_func;
    struct MM_Arena_Block* current_block;
    MM_u16 block_count;
    
    union
    {
        struct MM_Arena_Block;
        MM_Arena_Block block;
    };
} MM_Arena;

MM_API MM_Arena*
MM_Arena_Init(MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func)
{
    MM_Arena* arena = reserve_func(MM_ARENA_BLOCK_RESERVE_SIZE);
    commit_func(arena, MM_ARENA_PAGE_SIZE);
    
    if (arena != 0)
    {
        *arena = (MM_Arena){
            .reserve_func  = reserve_func,
            .commit_func   = commit_func,
            .free_func     = free_func,
            .current_block = &arena->block,
        };
        
        arena->block = (MM_Arena_Block){
            .next   = 0,
            .offset = sizeof(MM_Arena_Block),                // NOTE: offset is relative to the block
            .space  = MM_ARENA_PAGE_SIZE - sizeof(MM_Arena), // NOTE: space is relative to the reserve base
        };
        
    }
    
    return arena;
}

MM_API void*
MM_Arena_Push(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    MM_Arena_Block* block = arena->current_block;
    
    MM_u32 offset = alignment - (block->offset & ~(alignment - 1));
    
    if (block->space < offset + size)
    {
        if ((MM_umm)block->offset + (MM_umm)block->space < MM_ARENA_BLOCK_RESERVE_SIZE)
        {
            // NOTE: I don't think the extra parentheses are necessary, since a u64 + u32 + u32 add chain should never
            //       be turned into u64 + (u32 + u32) by any reasonable compiler. However, I am a bit paranoid.
            arena->commit_func(((MM_u8*)block + block->offset) + block->space, MM_ARENA_PAGE_SIZE);
            block->space += MM_ARENA_PAGE_SIZE;
        }
        else if (block->next != 0)
        {
            arena->current_block = block = block->next;
            block->space += (block->offset - sizeof(MM_Arena_Block));
            block->offset = sizeof(MM_Arena_Block);
        }
        else
        {
            block->next = arena->reserve_func(MM_ARENA_BLOCK_RESERVE_SIZE);
            arena->commit_func(block, MM_ARENA_PAGE_SIZE);
            
            block = block->next;
            *block = (MM_Arena_Block){
                .next = 0,
                .offset = sizeof(MM_Arena_Block),
                .space  = MM_ARENA_PAGE_SIZE - sizeof(MM_Arena_Block),
            };
        }
    }
    
    void* result = (MM_u8*)block + block->offset + offset;
    block->offset += offset + size;
    block->space  -= offset + size;
    
    return result;
}

MM_API MM_Arena_Marker
MM_Arena_GetMarker(MM_Arena* arena)
{
    return (MM_Arena_Marker)(((MM_u64)arena->block_count << 32) | arena->current_block->offset);
}

MM_API void
MM_Arena_PopBack(MM_Arena* arena, MM_Arena_Marker marker)
{
    MM_umm block_index  = (MM_u64)marker >> 32;
    MM_umm block_offset = (MM_u32)(MM_u64)marker;
    
    MM_Arena_Block* block = &arena->block;
    MM_umm i = 0;
    for (; i < block_index && block != 0; ++i, block = block->next);
    
    MM_ASSERT(i == block_offset);
    MM_ASSERT(block->offset >= block_offset);
    
    arena->current_block = block;
    arena->current_block->space  += block->offset - arena->current_block->offset;
    arena->current_block->offset  = block_offset;
}

MM_API void
MM_Arena_Clear(MM_Arena* arena)
{
    arena->current_block = &arena->block;
    arena->current_block->space += (arena->current_block->offset - sizeof(MM_Arena_Block));
    arena->current_block->offset = sizeof(MM_Arena_Block);
}

MM_API void
MM_Arena_Free(MM_Arena* arena)
{
    arena->free_func(arena);
}