typedef union Big_Num
{
    f64 flt_parts[2];
    
    struct
    {
        f64 low;
        f64 high;
    };
} Big_Num;

typedef union I128
{
    u64 parts[2];
    
    struct
    {
        u64 low;
        u64 high;
    };
} I128;

global Big_Num Big_0;

internal Big_Num BigNum_FromU64(u64 val);
internal Big_Num BigNum_FromF64(f64 val);
internal Big_Num BigNum_Truncate(Big_Num val);
internal Big_Num BigNum_IChangeWidth(Big_Num val);
internal Big_Num BigNum_FChangeWidth(Big_Num val);
internal Big_Num BigNum_IAdd(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_ISub(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_IMul(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_IDiv(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_FAdd(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_FSub(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_FMul(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_FDiv(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_Rem(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_Complement(Big_Num val, umm byte_size);
internal Big_Num BigNum_BitAnd(Big_Num v0, Big_Num v1);
internal Big_Num BigNum_BitOr(Big_Num v0, Big_Num v1);
internal Big_Num BigNum_BitXor(Big_Num v0, Big_Num v1);
internal Big_Num BigNum_LeftShift(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_RightShift(Big_Num v0, Big_Num v1, imm byte_size);
internal Big_Num BigNum_ARightShift(Big_Num v0, Big_Num v1, imm byte_size);
internal bool BigNum_IsEqual(Big_Num v0, Big_Num v1);
internal bool BigNum_IsStrictLess(Big_Num v0, Big_Num v1);
internal bool BigNum_IsStrictGreater(Big_Num v0, Big_Num v1);

internal umm BigNum_IEffectiveSize(Big_Num val);
internal umm BigNum_FEffectiveSize(Big_Num val);

internal u64 BigNum_ToU64(Big_Num val);

internal I128
I128_FromBigNum(Big_Num val)
{
    I128 result = {0};
    
    ASSERT(!"float");
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
I128_ToBigNum(I128 val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal I128
I128_ChangeWidth(I128 val, imm src_size, imm dst_size, bool should_extend_sign)
{
    I128 result = {0};
    
    ASSERT(!"float");
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FromU64(u64 val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FromF64(f64 val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_Truncate(Big_Num val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_IChangeWidth(Big_Num val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FChangeWidth(Big_Num val)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_IAdd(Big_Num v0, Big_Num v1, imm byte_size)
{
    ASSERT(byte_size == -1 || 
           byte_size > 0 && BigNum_IEffectiveSize(v0) <= (umm)byte_size && BigNum_IEffectiveSize(v1) <= (umm)byte_size);
    
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    // NOTE: Check if the last matching 0 comes before the last matching 1
    //        10101011001
    //        01011001011
    //  ~& => 000001xxxxx
    //   & => 00001xxxxxx
    u64 carry = ((~i0.low & ~i1.low) < (i0.low & i1.low));
    
    I128 result = {
        .low  = i0.low  + i1.low,
        .high = i0.high + i1.high + carry,
    };
    
    return I128_ToBigNum(I128_ChangeWidth(result, -1, byte_size, false));
}

internal Big_Num
BigNum_ISub(Big_Num v0, Big_Num v1, imm byte_size)
{
    ASSERT(byte_size == -1 || 
           byte_size > 0 && BigNum_IEffectiveSize(v0) <= (umm)byte_size && BigNum_IEffectiveSize(v1) <= (umm)byte_size);
    
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    i1.low  = ~i1.low + 1;
    i1.high = ~i1.high;
    
    // NOTE: Check if the last matching 0 comes before the last matching 1
    //        10101011001
    //        01011001011
    //  ~& => 000001xxxxx
    //   & => 00001xxxxxx
    //
    //        The two's complement operation can also overflow, but the only way
    //        for it to produce a carry is with an initial and final value of 0,
    //        implying carry can never be more than 1
    u64 carry = (i1.low == 0 || (~i0.low & ~i1.low) < (i0.low & i1.low));
    
    I128 result = {
        .low  = i0.low  + i1.low,
        .high = i0.high + i1.high + carry,
    };
    
    return I128_ToBigNum(I128_ChangeWidth(result, -1, byte_size, false));
}

internal Big_Num
BigNum_IMul(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_IDiv(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FAdd(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FSub(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FMul(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_FDiv(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_Rem(Big_Num v0, Big_Num v1, imm byte_size)
{
    Big_Num result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Num
BigNum_Complement(Big_Num val, umm byte_size)
{
    I128 i = I128_FromBigNum(val);
    
    I128 result = {
        .low  = ~i.low,
        .high = ~i.high,
    };
    
    return I128_ToBigNum(result);
}

internal Big_Num
BigNum_BitAnd(Big_Num v0, Big_Num v1)
{
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    I128 result = {
        .low  = i0.low  & i1.low,
        .high = i0.high & i1.high,
    };
    
    return I128_ToBigNum(result);
}

internal Big_Num
BigNum_BitOr(Big_Num v0, Big_Num v1)
{
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    I128 result = {
        .low  = i0.low  | i1.low,
        .high = i0.high | i1.high,
    };
    
    return I128_ToBigNum(result);
}

internal Big_Num
BigNum_BitXor(Big_Num v0, Big_Num v1)
{
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    I128 result = {
        .low  = i0.low  ^ i1.low,
        .high = i0.high ^ i1.high,
    };
    
    return I128_ToBigNum(result);
}

internal Big_Num
BigNum_LeftShift(Big_Num v0, Big_Num v1, imm byte_size)
{
    ASSERT(byte_size == -1 || 
           byte_size > 0 && BigNum_IEffectiveSize(v0) <= (umm)byte_size && BigNum_IEffectiveSize(v1) <= (umm)byte_size);
    
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    umm shift_amount = i1.low & 0x7F;
    
    // NOTE: case analysis for shifting low into high
    // 0  <= shift_amount <= 64   high |= low >> (64 - shift_amount), high |= low >> (64 - MIN(64, shift_amount))
    // 65 <= shift_amount <= 127  high |= low << (shift_amount - 64), high |= low << (MAX(64, shift_amount) - 64)
    
    I128 result = {
        .low  = i0.low  << shift_amount,
        .high = i0.high << shift_amount | (i0.low >> (64 - MIN(64, shift_amount))) << (MAX(64, shift_amount) - 64),
    };
    
    return I128_ToBigNum(I128_ChangeWidth(result, -1, byte_size, false));
}

internal Big_Num
BigNum_ARightShift(Big_Num v0, Big_Num v1, imm byte_size)
{
    ASSERT(byte_size == -1 || 
           byte_size > 0 && BigNum_IEffectiveSize(v0) <= (umm)byte_size && BigNum_IEffectiveSize(v1) <= (umm)byte_size);
    
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    I128 result;
    if (byte_size == -1)
    {
        umm shift_amount = i1.low & 0x7F;
        
        // NOTE: case analysis for shifting high into low
        // 0  <= shift_amount <= 64   low |= high << (64 - shift_amount), low |= high << (64 - MIN(64, shift_amount))
        // 65 <= shift_amount <= 127  low |= high >> (shift_amount - 64), low |= high >> (MAX(64, shift_amount) - 64)
        
        // NOTE: i0.high is casted to i64 to perform an arithmetic right shift instead of a logical shift
        //       this is done both for the high part, and the low adjustment part, to ensure the sign bit
        //       is properly propagated
        
        result = (I128){
            .low  = i0.low >> shift_amount | ((i64)i0.high >> (MAX(shift_amount, 64) - 64)) << (64 - MIN(64, shift_amount)),
            .high = (i64)i0.high >> shift_amount,
        };
    }
    
    // NOTE: there is probably a better way of doing this, but it does not matter much
    else if (byte_size == 8) result = (I128){ .low = (i64)i0.low >> (i1.low & 0x3F) };
    else if (byte_size == 4) result = (I128){ .low = (i32)i0.low >> (i1.low & 0x1F) };
    else if (byte_size == 2) result = (I128){ .low = (i16)i0.low >> (i1.low & 0x0F) };
    else                     result = (I128){ .low = (i8) i0.low >> (i1.low & 0x07) };
    
    return I128_ToBigNum(result);
}

internal Big_Num
BigNum_RightShift(Big_Num v0, Big_Num v1, imm byte_size)
{
    ASSERT(byte_size == -1 || 
           byte_size > 0 && BigNum_IEffectiveSize(v0) <= (umm)byte_size && BigNum_IEffectiveSize(v1) <= (umm)byte_size);
    
    I128 i0 = I128_FromBigNum(v0);
    I128 i1 = I128_FromBigNum(v1);
    
    umm shift_amount = i1.low & 0x7F;
    
    // NOTE: case analysis for shifting high into low
    // 0  <= shift_amount <= 64   low |= high << (64 - shift_amount), low |= high << (64 - MIN(64, shift_amount))
    // 65 <= shift_amount <= 127  low |= high >> (shift_amount - 64), low |= high >> (MAX(64, shift_amount) - 64)
    
    I128 result = {
        .low  = i0.low  >> shift_amount | (i0.high >> (MAX(shift_amount, 64) - 64)) << (64 - MIN(64, shift_amount)),
        .high = i0.high >> shift_amount,
    };
    
    return I128_ToBigNum(result);
}

internal bool
BigNum_IsEqual(Big_Num v0, Big_Num v1)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal bool
BigNum_IsStrictLess(Big_Num v0, Big_Num v1)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal bool
BigNum_IsStrictGreater(Big_Num v0, Big_Num v1)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal umm
BigNum_IEffectiveSize(Big_Num val)
{
    umm result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal umm
BigNum_FEffectiveSize(Big_Num val)
{
    umm result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal u64
BigNum_ToU64(Big_Num val)
{
    I128 i = I128_FromBigNum(val);
    
    ASSERT(i.high == 0);
    
    return i.low;
}