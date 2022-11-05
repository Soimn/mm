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
    
    return (new_hi_hi == 0 && c2 == 0 && (MM_i64)n->hi >= 0);
}