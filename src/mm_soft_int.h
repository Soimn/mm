typedef struct MM_Soft_Int
{
    union
    {
        MM_u32 eh[8];
        MM_u64 e[4];
        struct
        {
            MM_u128 lo;
            MM_u128 hi;
        };
    };
} MM_Soft_Int;

MM_Soft_Int
MM_SoftInt_FromU64(MM_u64 n)
{
    return (MM_Soft_Int){ .e[0] = n };
}

MM_u64
MM_SoftInt_ChopToU64(MM_Soft_Int n)
{
    return n.e[0];
}

MM_umm
MM_SoftInt_PartCount(MM_Soft_Int n)
{
    unsigned long mask = !!n.e[0] | (!!n.e[1] << 1) | (!!n.e[2] << 2) | (!!n.e[3] << 3);
    
    unsigned long largest_index;
    unsigned char non_zero = _BitScanReverse(&largest_index, mask);
    
    return largest_index + non_zero;
}

MM_umm
MM_SoftInt_HalfPartCount(MM_Soft_Int n)
{
    unsigned long mask = 0;
    for (MM_umm i = 0; i < 8; ++i) mask |= !!n.eh[0];
    
    unsigned long largest_index;
    unsigned char non_zero = _BitScanReverse(&largest_index, mask);
    
    return largest_index + non_zero;
}

typedef MM_u8 MM_Soft_Int_Status;
enum MM_SOFT_INT_STATUS
{
    MM_SoftIntStatus_Success = 0,
    
    MM_SoftIntStatus_OverflowBit  = 1,
    MM_SoftIntStatus_DivByZeroBit = 2,
};

MM_Soft_Int
MM_SoftInt_Neg(MM_Soft_Int n, MM_Soft_Int_Status* status)
{
    MM_Soft_Int r = {
        .e[0] = ~n.e[0] + 1,
        .e[1] = ~n.e[1] + (n.e[0] == 0),
        .e[2] = ~n.e[2] + (n.e[0] + n.e[1] == 0),
        .e[3] = ~n.e[3] + (n.e[0] + n.e[1] + n.e[2] == 0)
    };
    
    if (n.e[0] + n.e[1] + n.e[2] + n.e[3] == 0)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return r;
}

MM_bool
MM_SoftInt_IsNeg(MM_Soft_Int n)
{
    return ((MM_i64)n.e[3] < 0);
}

MM_bool
MM_SoftInt_IsZero(MM_Soft_Int n)
{
    return (n.lo == 0 && n.hi == 0);
}

MM_Soft_Int
MM_SoftInt_AddU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_u128 bb = (MM_u128)b;
    
    MM_u8 lo_carry = ((a.lo & bb) > (~a.lo & ~bb));
    
    MM_Soft_Int r = {
        .lo = a.lo + bb,
        .hi = a.hi + lo_carry,
    };
    
    if ((MM_i128)(~(a.hi ^ 0) & r.hi) < 0)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Add(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_u8 lo_carry = ((a.lo & b.lo) > (~a.lo & ~b.lo));
    MM_u8 hi_carry = ((a.hi & b.hi) > (~a.hi & ~b.hi));
    
    MM_u128 par_hi = a.hi + b.hi;
    
    MM_Soft_Int r = {
        .lo = a.lo + b.lo,
        .hi = par_hi + lo_carry,
    };
    
    if ((MM_i128)(~(a.hi ^ b.hi) & r.hi) < 0)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return r;
}

MM_Soft_Int
MM_SoftInt_SubU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_u128 bb = (MM_u128)b;
    
    MM_u8 lo_carry = ((a.lo & bb) > (~a.lo & ~bb)) | ((a.lo | bb) == MM_U128_MAX);
    
    MM_Soft_Int r = {
        .lo = a.lo + ~bb + 1,
        .hi = a.hi + MM_U128_MAX + lo_carry,
    };
    
    if ((MM_i128)((a.hi ^ 0) & ~(0 ^ r.hi)) < 0)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Sub(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_u8 lo_carry = ((a.lo & b.lo) > (~a.lo & ~b.lo)) | ((a.lo | b.lo) == MM_U128_MAX);
    
    MM_Soft_Int r = {
        .lo = a.lo + ~b.lo + 1,
        .hi = a.hi + ~b.lo + lo_carry,
    };
    
    if ((MM_i128)((a.hi ^ b.hi) & ~(b.hi ^ r.hi)) < 0)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return r;
}

MM_Soft_Int
MM_SoftInt_MulU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_bool should_flip = MM_false;
    
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a, &(MM_Soft_Int_Status){0});
        should_flip = MM_true;
    }
    
    MM_u128 a0b0, a0b1, a0b2, a0b3;
    MM_u128 a1b0, a1b1, a1b2;
    MM_u128 a2b0, a2b1;
    MM_u128 a3b0;
    
    *(MM_u64*)a0b0 = _umul128(a.e[0], b, (MM_u64*)&a0b0 + 1);
    
    *(MM_u64*)a1b0 = _umul128(a.e[1], b, (MM_u64*)&a1b0 + 1);
    
    *(MM_u64*)a2b0 = _umul128(a.e[2], b, (MM_u64*)&a2b0 + 1);
    
    *(MM_u64*)a3b0 = _umul128(a.e[3], b, (MM_u64*)&a3b0 + 1);
    
    MM_u128 r0 = a0b0;
    MM_u128 r1 = a1b0 + (r0 >> 64);
    MM_u128 r2 = a2b0 + (r1 >> 64);
    MM_u128 r3 = a3b0 + (r2 >> 64);
    
    MM_Soft_Int r = {
        .e[0] = (MM_u64)r0,
        .e[1] = (MM_u64)r1,
        .e[2] = (MM_u64)r2,
        .e[3] = (MM_u64)r3,
    };
    
    if (r3 >> 64 != 0 || should_flip && MM_SoftInt_IsNeg(r))
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return (should_flip ? MM_SoftInt_Neg(r, &(MM_Soft_Int_Status){0}) : r);
}

MM_Soft_Int
MM_SoftInt_Mul(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_bool should_flip = MM_false;
    
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a, &(MM_Soft_Int_Status){0});
        should_flip = MM_true;
    }
    
    if (MM_SoftInt_IsNeg(b))
    {
        b           = MM_SoftInt_Neg(b, &(MM_Soft_Int_Status){0});
        should_flip = !should_flip;
    }
    
    MM_u128 a0b0, a0b1, a0b2, a0b3;
    MM_u128 a1b0, a1b1, a1b2;
    MM_u128 a2b0, a2b1;
    MM_u128 a3b0;
    
    *(MM_u64*)a0b0 = _umul128(a.e[0], b.e[0], (MM_u64*)&a0b0 + 1);
    *(MM_u64*)a0b1 = _umul128(a.e[0], b.e[1], (MM_u64*)&a0b1 + 1);
    *(MM_u64*)a0b2 = _umul128(a.e[0], b.e[2], (MM_u64*)&a0b2 + 1);
    *(MM_u64*)a0b3 = _umul128(a.e[0], b.e[3], (MM_u64*)&a0b3 + 1);
    
    *(MM_u64*)a1b0 = _umul128(a.e[1], b.e[0], (MM_u64*)&a1b0 + 1);
    *(MM_u64*)a1b1 = _umul128(a.e[1], b.e[1], (MM_u64*)&a1b1 + 1);
    *(MM_u64*)a1b2 = _umul128(a.e[1], b.e[2], (MM_u64*)&a1b2 + 1);
    
    *(MM_u64*)a2b0 = _umul128(a.e[2], b.e[0], (MM_u64*)&a2b0 + 1);
    *(MM_u64*)a2b1 = _umul128(a.e[2], b.e[1], (MM_u64*)&a2b1 + 1);
    
    *(MM_u64*)a3b0 = _umul128(a.e[3], b.e[0], (MM_u64*)&a3b0 + 1);
    
    MM_u128 r0 = a0b0;
    MM_u128 r1 = a1b0 + a0b1 +               (r0 >> 64);
    MM_u128 r2 = a2b0 + a1b1 + a0b2 +        (r1 >> 64);
    MM_u128 r3 = a3b0 + a2b1 + a1b2 + a0b3 + (r2 >> 64);
    
    MM_Soft_Int r = {
        .e[0] = (MM_u64)r0,
        .e[1] = (MM_u64)r1,
        .e[2] = (MM_u64)r2,
        .e[3] = (MM_u64)r3,
    };
    
    MM_umm len_a = MM_SoftInt_PartCount(a);
    MM_umm len_b = MM_SoftInt_PartCount(b);
    
    if (r3 >> 64 != 0 || len_a + len_b > 3 || should_flip && MM_SoftInt_IsNeg(r))
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    return (should_flip ? MM_SoftInt_Neg(r, &(MM_Soft_Int_Status){0}) : r);
}

MM_Soft_Int
MM_SoftInt_Div(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int* remainder, MM_Soft_Int_Status* status)
{
    MM_bool flipped_a   = MM_false;
    MM_bool should_flip = MM_false;
    
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a, &(MM_Soft_Int_Status){0});
        should_flip = MM_true;
        flipped_a   = MM_true;
    }
    
    if (MM_SoftInt_IsNeg(b))
    {
        b           = MM_SoftInt_Neg(b, &(MM_Soft_Int_Status){0});
        should_flip = !should_flip;
    }
    
    MM_umm len_a = MM_SoftInt_HalfPartCount(a);
    MM_umm len_b = MM_SoftInt_HalfPartCount(b);
    
    MM_Soft_Int r;
    if (len_b == 0)
    {
        *status |= MM_SoftIntStatus_DivByZeroBit;
    }
    else if (len_b > len_a)
    {
        MM_ZeroStruct(&r);
        *remainder = b;
    }
    else if (len_b == 1)
    {
        MM_u64 rem = 0;
        for (MM_imm i = 7; i >= 0; --i)
        {
            r.eh[i] = ((rem << 32) | a.eh[i]) / b.e[0];
            rem     = a.eh[i] % b.e[0];
        }
        
        *remainder = (MM_Soft_Int){ .eh[0] = rem };
    }
    else
    {
        MM_NOT_IMPLEMENTED;
    }
    
    if (should_flip)
    {
        if (MM_SoftInt_IsNeg(r)) *status |= MM_SoftIntStatus_OverflowBit;
        else
        {
            r = MM_SoftInt_Neg(r, status);
            if (flipped_a) *remainder = MM_SoftInt_Neg(*remainder, status);
        }
    }
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Not(MM_Soft_Int val)
{
    return (MM_Soft_Int){
        .lo = ~val.lo,
        .hi = ~val.hi,
    };
}

MM_Soft_Int
MM_SoftInt_Or(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo | b.lo,
        .hi = a.hi | b.hi,
    };
}

MM_Soft_Int
MM_SoftInt_OrU64(MM_Soft_Int val, MM_u64 u64)
{
    MM_Soft_Int result = val;
    result.e[0] |= u64;
    return result;
}

MM_Soft_Int
MM_SoftInt_And(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo & b.lo,
        .hi = a.hi & b.hi,
    };
}

MM_Soft_Int
MM_SoftInt_AndU64(MM_Soft_Int val, MM_u64 u64)
{
    return (MM_Soft_Int){ .e[0] = val.e[0] & u64 };
}

MM_Soft_Int
MM_SoftInt_Xor(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo ^ b.lo,
        .hi = a.hi ^ b.hi,
    };
}

MM_Soft_Int
MM_SoftInt_XorU64(MM_Soft_Int val, MM_u64 u64)
{
    return (MM_Soft_Int){
        .lo = val.lo ^ u64,
        .hi = val.hi ^ 0,
    };
}

// NOTE: The bit shifting code is based on experimentation in compiler explorer with clang 14 on O2, trying to recreate the code generated for
//       builtin 128 bit arithmetic with 64 bit shifts. The following 256 bit procedures are based on the 64 bit variants.
//       link: https://godbolt.org/z/arvsK89P3

MM_Soft_Int
MM_SoftInt_Shl(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
    MM_u128 hi = a.hi << shift_amount | a.lo >> (128 - shift_amount);
    MM_u128 lo = a.lo << shift_amount;
    
    if (shift_amount & 128)
    {
        hi = lo;
        lo = 0;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_ShlU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Shl(val, (MM_Soft_Int){ .e[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Shr(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
    MM_u128 lo = a.lo >> shift_amount | a.hi << (128 - shift_amount);
    MM_u128 hi = a.hi >> shift_amount;
    
    if (shift_amount & 128)
    {
        lo = hi;
        hi= 0;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_ShrU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Shr(val, (MM_Soft_Int){ .e[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Sar(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
    MM_u128 lo = a.lo >> shift_amount | a.hi << (128 - shift_amount);
    MM_u128 hi = (MM_i128)a.hi >> shift_amount;
    
    if (shift_amount & 128)
    {
        lo = hi;
        hi = (MM_i128)a.hi >> 127;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_SarU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Sar(val, (MM_Soft_Int){ .e[0] = u64 });
}

MM_bool
MM_SoftInt_IsEqual(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi == b.hi && a.lo == b.lo);
}

MM_bool
MM_SoftInt_IsEqualU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo == u64);
}

MM_bool
MM_SoftInt_IsLess(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo));
}

MM_bool
MM_SoftInt_IsLessU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo < u64);
}

MM_bool
MM_SoftInt_IsLessEq(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi < b.hi || (a.hi == b.hi && a.lo <= b.lo));
}

MM_bool
MM_SoftInt_IsLessEqU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo <= u64);
}

MM_bool
MM_SoftInt_IsGreater(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi >= b.hi && (a.hi != b.hi || a.lo > b.lo));
}

MM_bool
MM_SoftInt_IsGreaterU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi != 0 || val.lo > u64);
}

MM_bool
MM_SoftInt_IsGreaterEq(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi >= b.hi && (a.hi != b.hi || a.lo >= b.lo));
}

MM_bool
MM_SoftInt_IsGreaterEqU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi != 0 || val.lo >= u64);
}