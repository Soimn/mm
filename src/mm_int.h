// IMPORTANT NOTE: This code is not optimized, I just tried to remove branches and reduce instruction count for fun

typedef union MM_i128
{
    MM_u32 e[4];
    struct
    {
        MM_u64 lo;
        MM_u64 hi;
    };
} MM_i128;

MM_i128
MM_i128_FromU64(MM_u64 u64)
{
    return (MM_i128){ .lo = u64 };
}

MM_bool
MM_i128_AppendDigit(MM_i128* n, MM_u8 base, MM_u8 digit)
{
    MM_ASSERT(base > 1);
    
    MM_u64 new_lo_hi, new_hi_hi;
    MM_u64 new_lo = _umul128(n->lo, base, &new_lo_hi);
    MM_u64 new_hi = _umul128(n->hi, base, &new_hi_hi);
    
    MM_u8 c1 = _addcarry_u64(0,  new_lo, digit,     &n->lo);
    MM_u8 c2 = _addcarry_u64(c1, new_hi, new_lo_hi, &n->hi);
    
    return (!new_hi_hi & !c2 & !(n->hi >> 63));
}

MM_i128
MM_i128_MaskToByteSize(MM_i128 n, MM_umm size)
{
    MM_ASSERT(MM_IS_POW_2(size) && size <= 16);
    
    // NOTE: __ull_rshift is used to ensure masking shift to 5 bits, this then behaves as MM_U64_MAX >> (64 - min(size*8, 64))
    MM_u64 lo_mask = __ull_rshift(MM_U64_MAX, -(size << 3));
    MM_u64 hi_mask = (size == 16 ? MM_U64_MAX : 0);
    
    return (MM_i128){
        .lo = n.lo & lo_mask,
        .hi = n.hi & hi_mask,
    };
}

MM_i128
MM_i128_SignExtendFromByteSize(MM_i128 n, MM_umm size)
{
    MM_ASSERT(MM_IS_POW_2(size) && size <= 16);
    
    MM_u64 sign_mask = __ll_rshift((size == 16 ? 0 : n.lo), 63);
    
    MM_u64 lo_ext = __ull_lshift(sign_mask, (size << 3) - 1);
    MM_u64 hi_ext = sign_mask;
    
    return (MM_i128){
        .lo = n.lo | lo_ext,
        .hi = n.hi | hi_ext,
    };
}

MM_i128
MM_i128_Sub(MM_i128 a, MM_i128 b)
{
    MM_i128 result = {0};
    MM_NOT_IMPLEMENTED;
    return result;
}

MM_imm
MM_i128_Cmp(MM_i128 a, MM_i128 b)
{
    MM_i128 res = MM_i128_Sub(a, b);
    
    return ((MM_i64)res.hi < 0 ? -1 : (res.lo == 0 && res.hi == 0 ? 0 : 1));
}

MM_i128
MM_i128_BitNot(MM_i128 n)
{
    return (MM_i128){
        .lo = ~n.lo,
        .hi = ~n.hi,
    };
}

MM_i128_LogicalNot(MM_i128 n)
{
    return (MM_i128){
        .lo = !n.lo,
        .hi = 0,
    };
}