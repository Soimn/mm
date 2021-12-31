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

#define CopyStruct(src, dst) Copy((src), (dst), sizeof(*(src)))

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

#define MEMORY_ARENA_PAGE_SIZE KB(4)

typedef struct Memory_Arena
{
    u64 base_address;
    u64 offset;
    u64 size;
} Memory_Arena;

internal void*
Arena_PushSize(Memory_Arena* arena, umm size, u8 alignment)
{
    u64 offset = arena->offset + AlignOffset((void*)(arena->base_address + arena->offset), alignment);
    
    if (offset + size > arena->size)
    {
        System_GrowArena(arena, offset + size - arena->size);
    }
    
    arena->offset = offset + size;
    
    return (void*)(arena->base_address + offset);
}

internal inline void
Arena_PopSize(Memory_Arena* arena, umm size)
{
    ASSERT(arena->offset > size);
    arena->offset -= size;
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

internal umm
Arena_UsedSize(Memory_Arena* arena)
{
    return arena->offset;
}

typedef struct DynArray
{
    Memory_Arena arena;
    u64 elem_size;
} DynArray;

internal DynArray
DynArray__Init(umm elem_size)
{
    DynArray array = { .elem_size = elem_size };
    System_InitArena(&array.arena);
    
    return array;
}

#define DynArray_Init(T) DynArray__Init(sizeof(T))

internal umm
DynArray_Size(DynArray* array)
{
    return Arena_UsedSize(&array->arena) / array->elem_size;
}

internal void*
DynArray_Push(DynArray* array)
{
    return Arena_PushSize(&array->arena, array->elem_size, 1);
}

internal void
DynArray_Pop(DynArray* array)
{
    Arena_PopSize(&array->arena, array->elem_size);
}

internal void*
DynArray_PushMulti(DynArray* array, umm element_count)
{
    return Arena_PushSize(&array->arena, array->elem_size * element_count, 1);
}

internal void
DynArray_PopMulti(DynArray* array, umm element_count)
{
    Arena_PopSize(&array->arena, array->elem_size * element_count);
}
