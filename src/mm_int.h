typedef struct MM_Soft_Int
{
    union
    {
        MM_u32 e[4];
        
        struct
        {
            MM_u64 lo;
            MM_u64 hi;
        };
    };
    MM_bool sign;
    MM_bool overflow; // NOTE: set when result of an operation which overflowed (or underflowed)
    MM_bool nan;      // NOTE: set when result of an operation with no defined result (e.g. n / 0)
} MM_Soft_Int;

MM_Soft_Int
MM_SoftInt_FromU64(MM_u64 u64)
{
    return (MM_Soft_Int){ .lo = u64 };
}


MM_Soft_Int
MM_SoftInt_FromI64(MM_i64 i64)
{
    return (MM_Soft_Int){ .lo = (MM_u64)i64, .hi = (i64 < 0 ? MM_U64_MAX : 0) };
}

#define MM_SOFT_INT_INIT_UNARY(a) (MM_Soft_Int){ .overflow = (a).overflow, .nan = (a).nan }
#define MM_SOFT_INT_INIT_BINARY(a, b) (MM_Soft_Int){ .overflow = ((a).overflow | (b).overflow), .nan = ((a).nan | (b).nan) }

MM_Soft_Int
MM_SoftInt_Add(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_u8 c0 = _addcarry_u64(0,  a.lo, b.lo, &r.lo);
    MM_u8 c1 = _addcarry_u64(c0, a.hi, b.hi, &r.hi);
    
    r.sign      = !!(a.sign + b.sign + c1);
    r.overflow |= (a.sign == b.sign && a.sign != r.sign);
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Sub(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_u64 sign;
    
    MM_u8 b0 = _subborrow_u64(0,  a.lo, b.lo, &r.lo);
    MM_u8 b1 = _subborrow_u64(b0, a.hi, b.hi, &r.hi);
    MM_u8 b2 = _subborrow_u64(b1, a.sign, b.sign, &sign);
    
    r.sign      = sign & 1;
    r.overflow |= (b2 != 0);
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitOr(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    r.lo   = a.lo   | b.lo;
    r.hi   = a.hi   | b.hi;
    r.sign = a.sign | b.sign;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitXor(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    r.lo   = a.lo   ^ b.lo;
    r.hi   = a.hi   ^ b.hi;
    r.sign = a.sign ^ b.sign;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Mul(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_NOT_IMPLEMENTED;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_Div(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int* remainder)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_NOT_IMPLEMENTED;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitAnd(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    r.lo   = a.lo   & b.lo;
    r.hi   = a.hi   & b.hi;
    r.sign = a.sign & b.sign;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitShl(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    //MM_u8 shift = b.e[0] & ;
    MM_NOT_IMPLEMENTED;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitShr(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_NOT_IMPLEMENTED;
    
    return r;
}

MM_Soft_Int
MM_SoftInt_BitSar(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_Soft_Int r = MM_SOFT_INT_INIT_BINARY(a, b);
    
    MM_NOT_IMPLEMENTED;
    
    return r;
}