#define MM_ROUND_UP(n, align) (((n) + ((align) - 1)) & ~((align) - 1))

MM_umm
MM_RoundUp(MM_umm n, MM_umm align)
{
    return MM_ROUND_UP(n, align);
}

#define MM_ROUND_DOWN(n, align) ((n) & ~((align) - 1))

MM_umm
MM_RoundDown(MM_umm n, MM_umm align)
{
    return MM_ROUND_DOWN(n, align);
}

void*
MM_Align(void* ptr, MM_u8 alignment)
{
    return (void*)MM_ROUND_UP((MM_umm)ptr, (MM_umm)alignment);
}

MM_u8
MM_AlignOffset(void* ptr, MM_u8 alignment)
{
    return (MM_u8)((MM_u8*)MM_ROUND_UP((MM_umm)ptr, (MM_umm)alignment) - (MM_u8*)ptr);
}

void
MM_Copy(void* dst, void* src, MM_umm size)
{
    MM_u8* bsrc = (MM_u8*)src;
    MM_u8* bdst = (MM_u8*)dst;
    for (MM_umm i = 0; i < size; ++i) bdst[i] = bsrc[i];
}

void
MM_Move(void* dst, void* src, MM_umm size)
{
    MM_u8* bsrc = (MM_u8*)src;
    MM_u8* bdst = (MM_u8*)dst;
    
    if (bsrc > bdst || bsrc + size <= bdst) MM_Copy(dst, src, size);
    else
    {
        for (MM_umm i = 0, j = size; i < size; ++i, --j) bdst[j] = bsrc[j];
    }
}

void
MM_Zero(void* ptr, MM_umm size)
{
    for (MM_umm i = 0; i < size; ++i)
    {
        ((MM_u8*)ptr)[i] = 0;
    }
}

#define MM_ZeroStruct(s) MM_Zero((s), sizeof(*(s)))

typedef struct MM_Arena
{
    MM_u64 offset;
    MM_u64 space;
    MM_u64 page_size;
    MM_u8 mem_start[];
} MM_Arena;

typedef MM_u64 MM_Arena_Marker;

MM_Arena*
MM_Arena_Init(MM_umm page_size, MM_umm reserve_size)
{
    MM_ASSERT(page_size % MM_SYSTEM_PAGE_SIZE == 0);
    MM_ASSERT(reserve_size > 0 && reserve_size % page_size == 0);
    
    MM_Arena* arena = MM_SYSTEM_RESERVE_MEMORY(reserve_size);
    MM_SYSTEM_COMMIT_MEMORY(arena, page_size);
    
    *arena = (MM_Arena){
        .offset    = 0,
        .space     = page_size - sizeof(MM_Arena),
        .page_size = page_size,
    };
    
    return arena;
}

void*
MM_Arena_Push(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    MM_ASSERT(size <= MM_U64_MAX / 2);
    
    MM_u8 align_offset = MM_AlignOffset(arena->mem_start + arena->offset, alignment);
    
    if (arena->space < size + align_offset)
    {
        MM_SYSTEM_COMMIT_MEMORY(arena->mem_start + arena->offset + arena->space, arena->page_size);
        arena->space += arena->page_size;
    }
    
    void* result = arena->mem_start + arena->offset + align_offset;
    
    MM_umm adjustment = align_offset + size;
    arena->offset += adjustment;
    arena->space  -= adjustment;
    
    return result;
}

void*
MM_Arena_PushCopy(MM_Arena* arena, void* data, MM_umm size, MM_u8 alignment)
{
    void* new_data = MM_Arena_Push(arena, size, alignment);
    MM_Copy(new_data, data, size);
    
    return new_data;
}

void*
MM_Arena_PushZero(MM_Arena* arena, MM_umm size, MM_u8 alignment)
{
    void* data = MM_Arena_Push(arena, size, alignment);
    MM_Zero(data, size);
    
    return data;
}

void
MM_Arena_Pop(MM_Arena* arena, MM_umm size)
{
    MM_ASSERT((MM_imm)arena->offset - sizeof(MM_Arena) >= size);
    
    arena->offset -= size;
    arena->space  += size;
}

void
MM_Arena_Clear(MM_Arena* arena)
{
    arena->space += arena->offset - sizeof(MM_Arena);
    arena->offset = sizeof(MM_Arena);
}

void
MM_Arena_Free(MM_Arena** arena)
{
    MM_SYSTEM_FREE_MEMORY(*arena);
    *arena = 0;
}

MM_Arena_Marker
MM_Arena_GetMarker(MM_Arena* arena)
{
    return (MM_Arena_Marker)arena->offset;
}

void
MM_Arena_PopToMarker(MM_Arena* arena, MM_Arena_Marker marker)
{
    MM_ASSERT(arena->offset >= marker);
    
    arena->space += arena->offset - marker;
    arena->offset = marker;
}

MM_umm
MM_Arena_UsedBytes(MM_Arena* arena)
{
    return arena->offset;
}