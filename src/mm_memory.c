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
    struct MM_Arena_Block* prev;
    MM_u32 offset;
    MM_u32 space;
} MM_Arena_Block;

#define MM_ARENA_PAGE_SIZE MM_KB(16)
typedef struct MM_Arena
{
    MM_Reserve_Memory_Func reserve_func;
    MM_Commit_Memory_Func commit_func;
    MM_Free_Memory_Func free_func;
    struct MM_Arena_Block* current_block;
    MM_u16 block_count;
    MM_u16 current_block_index;
    MM_u32 block_size;
    
    union
    {
        struct MM_Arena_Block;
        MM_Arena_Block block;
    };
} MM_Arena;

MM_API MM_Arena*
MM_Arena_Init(MM_Reserve_Memory_Func reserve_func, MM_Commit_Memory_Func commit_func, MM_Free_Memory_Func free_func, MM_u32 block_size)
{
    block_size = MM_ROUND_UP(block_size, MM_ARENA_PAGE_SIZE);
    
    MM_Arena* arena = reserve_func(block_size);
    commit_func(arena, MM_ARENA_PAGE_SIZE);
    
    if (arena != 0)
    {
        *arena = (MM_Arena){
            .reserve_func        = reserve_func,
            .commit_func         = commit_func,
            .free_func           = free_func,
            .current_block       = &arena->block,
            .block_count         = 0,
            .current_block_index = 0,
            .block_size          = block_size,
        };
        
        arena->block = (MM_Arena_Block){
            .next   = 0,
            .prev   = 0,
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
    
    MM_u32 offset = MM_ROUND_UP(block->offset, alignment) - block->offset;
    
    if (block->space < offset + size)
    {
        if ((MM_umm)block->offset + (MM_umm)block->space < arena->block_size)
        {
            arena->commit_func(((MM_u8*)block + block->offset) + block->space, MM_ARENA_PAGE_SIZE);
            block->space += MM_ARENA_PAGE_SIZE;
        }
        else if (block->next != 0)
        {
            arena->current_block = block = block->next;
            block->space += (block->offset - sizeof(MM_Arena_Block));
            block->offset = sizeof(MM_Arena_Block);
            
            arena->current_block_index += 1;
        }
        else
        {
            block->next = arena->reserve_func(arena->block_size);
            arena->commit_func(block, MM_ARENA_PAGE_SIZE);
            
            *(block->next) = (MM_Arena_Block){
                .next   = 0,
                .prev   = block,
                .offset = sizeof(MM_Arena_Block),
                .space  = MM_ARENA_PAGE_SIZE - sizeof(MM_Arena_Block),
            };
            
            block                       = block->next;
            arena->current_block_index += 1;
        }
    }
    
    void* result = (MM_u8*)block + block->offset + offset;
    block->offset += offset + size;
    block->space  -= offset + size;
    
    return result;
}

MM_API void
MM_Arena_Pop(MM_Arena* arena, MM_umm amount)
{
    for (MM_Arena_Block* block = arena->current_block; amount > 0 && block != 0; block = block->prev, --arena->current_block_index)
    {
        MM_umm bite = MM_MIN(amount, block->offset - sizeof(MM_Arena_Block));
        block->offset -= bite;
        block->space  += bite;
        amount        -= bite;
    }
    
    MM_ASSERT(amount == 0);
}

MM_API MM_Arena_Marker
MM_Arena_GetMarker(MM_Arena* arena)
{
    return (MM_Arena_Marker)(((MM_u64)arena->current_block_index << 32) | arena->current_block->offset);
}

MM_API void
MM_Arena_PopBack(MM_Arena* arena, MM_Arena_Marker marker)
{
    MM_umm block_index  = (MM_u64)marker >> 32;
    MM_umm block_offset = (MM_u32)(MM_u64)marker;
    
    MM_Arena_Block* block = &arena->block;
    MM_umm i = 0;
    for (; i < block_index && block != 0; ++i, block = block->next)
    {
        MM_ASSERT(block != arena->current_block);
    }
    
    MM_ASSERT(i == block_offset);
    MM_ASSERT(block->offset >= block_offset);
    
    arena->current_block = block;
    arena->current_block->space  += block->offset - arena->current_block->offset;
    arena->current_block->offset  = block_offset;
    arena->current_block_index    = block_index;
}

MM_API void
MM_Arena_Clear(MM_Arena* arena)
{
    arena->current_block = &arena->block;
    arena->current_block->space += (arena->current_block->offset - sizeof(MM_Arena_Block));
    arena->current_block->offset = sizeof(MM_Arena_Block);
    arena->current_block_index   = 0;
}

MM_API void
MM_Arena_Free(MM_Arena* arena)
{
    for (MM_Arena_Block* block = arena->block.next; block != 0; )
    {
        MM_Arena_Block* next = block->next;
        
        arena->free_func(block);
        
        block = next;
    }
    
    arena->free_func(arena);
}

MM_API void
MM_Arena_FreeEveryBlockAfterCurrent(MM_Arena* arena)
{
    for (MM_Arena_Block* block = arena->current_block->next; block != 0; )
    {
        MM_Arena_Block* next = block->next;
        
        arena->free_func(block);
        
        arena->block_count -= 1;
        block = next;
    }
    
    arena->current_block->next = 0;
}