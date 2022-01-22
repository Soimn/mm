typedef union I128
{
    u32 p[4];
    
    struct
    {
        u64 lo;
        u64 hi;
    };
} I128;

internal inline I128
I128_BitShl(I128 v, u64 shift_amount)
{
    shift_amount = shift_amount & 0x7F;
    
    return (I128){
        .lo = v.lo << shift_amount,
        .hi = (v.hi << shift_amount) | (v.lo >> (64 - shift_amount)),
    };
}

internal inline I128
I128_BitShr(I128 v, u64 shift_amount)
{
    shift_amount = shift_amount & 0x7F;
    
    return (I128){
        .lo = (v.lo >> shift_amount) | (v.hi << (64 - shift_amount)),
        .hi = v.hi >> shift_amount,
    };
}

internal inline I128
I128_BitSar(I128 v, u64 shift_amount)
{
    shift_amount = shift_amount & 0x7F;
    
    return (I128){
        .lo = (v.lo >> shift_amount) | ((i64)v.hi << (64 - shift_amount)),
        .hi = (i64)v.hi >> shift_amount,
    };
}

internal inline I128
I128_BitOr(I128 v0, I128 v1)
{
    return (I128){
        .lo = v0.lo | v1.lo,
        .hi = v0.hi | v1.hi,
    };
}

internal inline I128
I128_BitXor(I128 v0, I128 v1)
{
    return (I128){
        .lo = v0.lo ^ v1.lo,
        .hi = v0.hi ^ v1.hi,
    };
}

internal inline I128
I128_BitAnd(I128 v0, I128 v1)
{
    return (I128){
        .lo = v0.lo & v1.lo,
        .hi = v0.hi & v1.hi,
    };
}

internal inline I128
I128_BitNot(I128 v)
{
    return (I128){
        .lo = ~v.lo,
        .hi = ~v.hi,
    };
}

internal inline I128
I128_Add(I128 v0, I128 v1)
{
    I128 result;
    
    _addcarryx_u64(_addcarryx_u64(0, v0.lo, v1.lo, &result.lo), v0.hi, v1.hi, &result.hi);
    
    return result;
}

internal inline I128
I128_Neg(I128 v)
{
    I128 result;
    
    _addcarryx_u64(_addcarryx_u64(0, ~v.lo, 1, &result.lo), ~v.hi, 0, &result.hi);
    
    return result;
}

internal inline I128
I128_Sub(I128 v0, I128 v1)
{
    return I128_Add(v0, I128_Neg(v1));
}

internal inline bool
I128_IsStrictlyLess(I128 v0, I128 v1)
{
    return (v0.hi < v1.hi || v0.hi == v1.hi && v0.lo < v1.lo);
}

internal inline bool
I128_IsStrictlyGreater(I128 v0, I128 v1)
{
    return (v0.hi > v1.hi || v0.hi == v1.hi && v0.lo > v1.lo);
}

internal inline bool
I128_IsLess(I128 v0, I128 v1)
{
    return !I128_IsStrictlyGreater(v0, v1);
}

internal inline bool
I128_IsGreater(I128 v0, I128 v1)
{
    return !I128_IsStrictlyLess(v0, v1);
}

internal inline bool
I128_IsEqual(I128 v0, I128 v1)
{
    return StructCompare(&v0, &v1);
}

internal inline I128
I128_Mul(I128 v0, I128 v1)
{
    u64 s0 = (u64)((i64)v0.hi >> 63);
    u64 s1 = (u64)((i64)v1.hi >> 63);
    
    v0.lo = (v0.lo ^ s0) - s0;
    v0.hi = (v0.hi ^ s0) - s0;
    
    v1.lo = (v1.lo ^ s1) - s1;
    v1.hi = (v1.hi ^ s1) - s1;
    
    s0 = s0 ^ s1;
    
    I128 r;
    
    r.lo  = _mul128(v0.lo, v1.lo, (i64*)&r.hi);
    r.hi += v0.hi*v1.lo + v0.lo*v1.hi;
    
    r.lo = (r.lo ^ s0) - s0;
    r.hi = (r.hi ^ s0) - s0;
    
    return r;
}

internal I128
I128_DivMod(I128 u, I128 v, I128* r)
{
    ASSERT(v.lo != 0 || v.hi != 0);
    
    I128 q = {0};
    
    u64 s0 = (u64)((i64)u.hi >> 63);
    u64 s1 = (u64)((i64)v.hi >> 63);
    
    u.lo = (u.lo ^ s0) - s0;
    u.hi = (u.hi ^ s0) - s0;
    
    v.lo = (v.lo ^ s1) - s1;
    v.hi = (v.hi ^ s1) - s1;
    
    s0 = s0 ^ s1;
    
    umm m = __lzcnt64(u.hi) + (u.hi == 0 ? __lzcnt64(u.lo) : 0);
    umm n = __lzcnt64(v.hi) + (v.hi == 0 ? __lzcnt64(v.lo) : 0);
    
    if (n > m) *r = u;
    else
    {
        umm distance = m - n;
        
        v = I128_BitShl(v, distance);
        
        for (imm i = distance; i >= 0; --i)
        {
            q = I128_BitShl(q, 1);
            
            if (I128_IsGreater(u, v))
            {
                I128_Sub(u, v);
                
                q.lo |= 1;
            }
            
            I128_BitShr(v, 1);
        }
        
        *r = u;
    }
    
    q.lo = (q.lo ^ s0) - s0;
    q.hi = (q.hi ^ s0) - s0;
    
    return q;
}

typedef struct Big_Int
{
    I128 val;
} Big_Int;

global Big_Int BigInt_0;

internal inline Big_Int
BigInt_FromU64(u64 val)
{
    return (Big_Int){ .val.lo = val };
}

internal inline Big_Int
BigInt_FromI64(i64 val)
{
    return (Big_Int){
        .val.lo = val,
        .val.hi = val >> 63,
    };
}

internal inline u64
BigInt_ToU64(Big_Int val)
{
    return val.val.lo;
}

internal inline i64
BigInt_ToI64(Big_Int val)
{
    return val.val.lo;
}

internal inline Big_Int
BigInt_BitShl(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitShl(v0.val, v1.val.lo) };
}

internal inline Big_Int
BigInt_BitShr(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitShr(v0.val, v1.val.lo) };
}

internal inline Big_Int
BigInt_BitSar(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitSar(v0.val, v1.val.lo) };
}

internal inline Big_Int
BigInt_BitOr(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitOr(v0.val, v1.val) };
}

internal inline Big_Int
BigInt_BitXor(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitXor(v0.val, v1.val) };
}

internal inline Big_Int
BigInt_BitAnd(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_BitAnd(v0.val, v1.val) };
}

internal inline Big_Int
BigInt_BitNot(Big_Int v)
{
    return (Big_Int){ I128_BitNot(v.val) };
}

internal inline Big_Int
BigInt_Add(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_Add(v0.val, v1.val) };
}

internal inline Big_Int
BigInt_Neg(Big_Int v)
{
    return (Big_Int){ I128_Neg(v.val) };
}

internal inline Big_Int
BigInt_Sub(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_Add(v0.val, I128_Neg(v1.val)) };
}

internal inline bool
BigInt_IsStrictlyLess(Big_Int v0, Big_Int v1)
{
    return (v0.val.hi < v1.val.hi || v0.val.hi == v1.val.hi && v0.val.lo < v1.val.lo);
}

internal inline bool
BigInt_IsStrictlyGreater(Big_Int v0, Big_Int v1)
{
    return (v0.val.hi > v1.val.hi || v0.val.hi == v1.val.hi && v0.val.lo > v1.val.lo);
}

internal inline bool
BigInt_IsLess(Big_Int v0, Big_Int v1)
{
    return !I128_IsStrictlyGreater(v0.val, v1.val);
}

internal inline bool
BigInt_IsGreater(Big_Int v0, Big_Int v1)
{
    return !I128_IsStrictlyLess(v0.val, v1.val);
}

internal inline bool
BigInt_IsEqual(Big_Int v0, Big_Int v1)
{
    return !I128_IsEqual(v0.val, v1.val);
}

internal inline Big_Int
BigInt_Mul(Big_Int v0, Big_Int v1)
{
    return (Big_Int){ I128_Mul(v0.val, v1.val) };
}

internal inline Big_Int
BigInt_DivMod(Big_Int v0, Big_Int v1, Big_Int* r)
{
    return (Big_Int){ I128_DivMod(v0.val, v1.val, (I128*)r) };
}

// HACK, TODO: Implement proper Big_Float logic
typedef struct Big_Float
{
    f64 val;
} Big_Float;

global Big_Float BigFloat_0;

internal inline Big_Float
BigFloat_FromF64(f64 val)
{
    return (Big_Float){ val };
}

internal inline Big_Float
BigFloat_Add(Big_Float v0, Big_Float v1)
{
    return (Big_Float){ v0.val + v1.val };
}

internal inline Big_Float
BigFloat_Neg(Big_Float v)
{
    return (Big_Float){ -v.val };
}

internal inline Big_Float
BigFloat_Sub(Big_Float v0, Big_Float v1)
{
    return (Big_Float){ v0.val - v1.val };
}

internal inline bool
BigFloat_IsStrictlyLess(Big_Float v0, Big_Float v1)
{
    return (v0.val < v1.val);
}

internal inline bool
BigFloat_IsStrictlyGreater(Big_Float v0, Big_Float v1)
{
    return (v0.val > v1.val);
}

internal inline bool
BigFloat_IsLess(Big_Float v0, Big_Float v1)
{
    return (v0.val <= v1.val);
}

internal inline bool
BigFloat_IsGreater(Big_Float v0, Big_Float v1)
{
    return (v0.val >= v1.val);
}

internal inline bool
BigFloat_IsEqual(Big_Float v0, Big_Float v1)
{
    return (v0.val == v1.val);
}

internal inline Big_Float
BigFloat_Mul(Big_Float v0, Big_Float v1)
{
    return (Big_Float){ v0.val * v1.val };
}

internal inline Big_Float
BigFloat_Div(Big_Float v0, Big_Float v1)
{
    return (Big_Float){ v0.val / v1.val };
}

internal inline Big_Float
BigFloat_FromBigInt(Big_Int val)
{
    return (Big_Float){ (f64)val.val.hi * ((u64)1 << 32) + val.val.lo };
}

// HACK: Replace this
internal inline Big_Float
BigFloat_Truncate(Big_Float val)
{
    union { u64 i; f64 f; } bits = { .f = val.val };
    
    i64 exponent = ((bits.i & 0x7FF0000000000000) >> 52) - 1023;
    
    bits.i &= ~(0x000FFFFFFFFFFFFF >> MIN(MAX(exponent, 0), 52));
    
    return (Big_Float){ bits.f };
}

internal inline Big_Float
BigFloat_FromU64(u64 val)
{
    return (Big_Float){ (f64)val };
}

internal inline Big_Float
BigFloat_FromI64(i64 val)
{
    return (Big_Float){ (f64)val };
}

internal inline i64
BigFloat_ToI64(Big_Float val)
{
    return (i64)val.val;
}

internal inline u64
BigFloat_ToU64(Big_Float val)
{
    return (u64)val.val;
}