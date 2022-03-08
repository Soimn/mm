typedef struct Big_Int
{
    u64 parts[4];
} Big_Int;

global Big_Int BigInt_0       = {0};
global Big_Int BigInt_1       = { .parts[0] = 1 };
global Big_Int BigInt_U64_MAX = { .parts[0] = U64_MAX };

internal inline Big_Int
BigInt_FromU64(u64 val)
{
    return (Big_Int){
        .parts[0] = val,
    };
}

internal inline umm
BigInt_ByteSize(Big_Int val)
{
    umm result = 1;
    
    if ((val.parts[3] != 0 + val.parts[2] != 0) != 0) result = 32;
    if (val.parts[1]                            != 0) result = 16;
    if ((val.parts[0] & 0xFFFFFFFF00000000)     != 0) result = 8;
    if ((val.parts[0] & 0x00000000FFFF0000)     != 0) result = 4;
    if ((val.parts[0] & 0x000000000000FF00)     != 0) result = 2;
    
    return result;
}

internal inline u64
BigInt_ToU64(Big_Int val)
{
    ASSERT(BigInt_ByteSize(val) <= 8);
    
    return val.parts[0];
}

internal Big_Int
BigInt_ChopToSize(Big_Int val, imm byte_size)
{
    ASSERT(byte_size == -1 || IS_POW_OF_2(byte_size) && byte_size <= 16);
    
    Big_Int result;
    result.parts[0] = val.parts[0] & (((u64)1 << ((u64)byte_size*8)) - 1);
    result.parts[1] = val.parts[1] & ((u64)byte_size >= 16 ? U64_MAX : 0);
    result.parts[2] = val.parts[2] & (     byte_size == -1 ? U64_MAX : 0);
    result.parts[3] = val.parts[3] & (     byte_size == -1 ? U64_MAX : 0);
    
    return result;
}

// NOTE: This is why x64 inline assembly should be supported
internal Big_Int
BigInt_SignExtend(Big_Int val, imm byte_size)
{
    ASSERT(byte_size == -1 || IS_POW_OF_2(byte_size) && byte_size <= 16);
    
    i64 ext_8  = (i8) val.parts[0];
    i64 ext_16 = (i16)val.parts[0];
    i64 ext_32 = (i32)val.parts[0];
    
    i64 lo = val.parts[0];
    if (byte_size == 1) lo = ext_8;
    if (byte_size == 2) lo = ext_16;
    if (byte_size == 4) lo = ext_32;
    
    i64 mi = val.parts[1];
    if ((((u64)byte_size < 16) & (lo < 0)) != 0) mi = -1;
    
    u64 hi_mask = 0;
    if ((byte_size != -1) & (mi < 0)) hi_mask = U64_MAX;
    
    return (Big_Int){
        .parts[0] = (u64)lo,
        .parts[1] = mi,
        .parts[2] = val.parts[2] | hi_mask,
        .parts[3] = val.parts[3] | hi_mask,
    };
}

internal inline Big_Int
BigInt_Neg(Big_Int val)
{
    Big_Int result;
    
    u8 c0 = _subborrow_u64(0, 0, val.parts[0], &result.parts[0]);
    u8 c1 = _subborrow_u64(c0, 0, val.parts[1], &result.parts[1]);
    u8 c2 = _subborrow_u64(c1, 0, val.parts[2], &result.parts[2]);
    _subborrow_u64(c2, 0, val.parts[3], &result.parts[3]);
    
    return result;
}

internal inline Big_Int
BigInt_Add(Big_Int a, Big_Int b)
{
    Big_Int result;
    
    u8 c0 = _addcarry_u64(0, a.parts[0], b.parts[0], &result.parts[0]);
    u8 c1 = _addcarry_u64(c0, a.parts[1], b.parts[1], &result.parts[1]);
    u8 c2 = _addcarry_u64(c1, a.parts[2], b.parts[2], &result.parts[2]);
    _addcarry_u64(c2, a.parts[3], b.parts[3], &result.parts[3]);
    
    return result;
}

internal inline Big_Int
BigInt_Sub(Big_Int a, Big_Int b)
{
    Big_Int result;
    
    u8 c0 = _subborrow_u64(0, a.parts[0], b.parts[0], &result.parts[0]);
    u8 c1 = _subborrow_u64(c0, a.parts[1], b.parts[1], &result.parts[1]);
    u8 c2 = _subborrow_u64(c1, a.parts[2], b.parts[2], &result.parts[2]);
    _subborrow_u64(c2, a.parts[3], b.parts[3], &result.parts[3]);
    
    return result;
}

internal inline bool
BigInt_IsGreater(Big_Int a, Big_Int b)
{
    Big_Int sub = BigInt_Sub(b, a);
    return ((i64)sub.parts[3] < 0);
}

internal inline bool
BigInt_IsLess(Big_Int a, Big_Int b)
{
    Big_Int sub = BigInt_Sub(a, b);
    return ((i64)sub.parts[3] < 0);
}

internal inline bool
BigInt_IsEqual(Big_Int a, Big_Int b)
{
    return Memcmp(&a, &b, sizeof(Big_Int));
}

// TODO: _mul128 is a MSVC intrinsic, replace with something else
internal inline Big_Int
BigInt_Mul(Big_Int a, Big_Int b)
{
    Big_Int result = {0};
    u64 p0_hi;
    u64 p0_lo = _mul128(a.parts[0], b.parts[0], (LONG64*)&p0_hi);
    
    u64 p10_hi, p11_hi;
    u64 p10_lo = _mul128(a.parts[1], b.parts[0], (LONG64*)&p10_hi);
    u64 p11_lo = _mul128(a.parts[0], b.parts[1], (LONG64*)&p11_hi);
    
    u64 p20_hi, p21_hi, p22_hi;
    u64 p20_lo = _mul128(a.parts[2], b.parts[0], (LONG64*)&p20_hi);
    u64 p21_lo = _mul128(a.parts[1], b.parts[1], (LONG64*)&p21_hi);
    u64 p22_lo = _mul128(a.parts[0], b.parts[2], (LONG64*)&p22_hi);
    
    u64 p30_hi, p31_hi, p32_hi, p33_hi;
    u64 p30_lo = _mul128(a.parts[3], b.parts[0], (LONG64*)&p30_hi);
    u64 p31_lo = _mul128(a.parts[2], b.parts[1], (LONG64*)&p31_hi);
    u64 p32_lo = _mul128(a.parts[1], b.parts[2], (LONG64*)&p32_hi);
    u64 p33_lo = _mul128(a.parts[0], b.parts[3], (LONG64*)&p33_hi);
    
    result.parts[0] = p0_lo;
    
    u64 r10;
    u8 c10 = _addcarry_u64(0, p0_hi, p10_lo, &r10);
    u8 c11 = _addcarry_u64(0, r10, p11_lo, &result.parts[1]);
    u8 c1 = c10 + c11;
    
    u64 r20, r21, r22, r23;
    u8 c20 = _addcarry_u64(0, c1, p10_hi, &r20);
    u8 c21 = _addcarry_u64(0, p11_hi, p20_lo, &r21);
    u8 c22 = _addcarry_u64(0, p21_lo, p22_lo, &r22);
    u8 c23 = _addcarry_u64(0, r20, r21, &r23);
    u8 c24 = _addcarry_u64(0, r23, r22, &result.parts[2]);
    u8 c2 = c20 + c21 + c22 + c23 + c24;
    
    result.parts[3] = (c2 + p20_hi + p21_hi + p22_hi + p30_lo + p31_lo + p32_lo + p33_lo);
    
    return result;
}

internal inline Big_Int
BigInt_DivMod(Big_Int a, Big_Int b, Big_Int* rem)
{
    Big_Int quotient = {0};
    
    // TODO: Replace this, b = 1 will be a nightmare
    while (BigInt_IsLess(a, b))
    {
        a = BigInt_Sub(a, b);
        quotient = BigInt_Add(quotient, BigInt_1);
    }
    
    *rem = a;
    
    return quotient;
}

internal inline Big_Int
BigInt_BitNot(Big_Int v)
{
    return (Big_Int){
        .parts[0] = ~v.parts[0],
        .parts[1] = ~v.parts[1],
        .parts[2] = ~v.parts[2],
        .parts[3] = ~v.parts[3],
    };
}

internal inline Big_Int
BigInt_BitAnd(Big_Int v0, Big_Int v1)
{
    return (Big_Int){
        .parts[0] = v0.parts[0] & v0.parts[0],
        .parts[1] = v0.parts[1] & v0.parts[1],
        .parts[2] = v0.parts[2] & v0.parts[2],
        .parts[3] = v0.parts[3] & v0.parts[3],
    };
}

internal inline Big_Int
BigInt_BitOr(Big_Int v0, Big_Int v1)
{
    return (Big_Int){
        .parts[0] = v0.parts[0] | v0.parts[0],
        .parts[1] = v0.parts[1] | v0.parts[1],
        .parts[2] = v0.parts[2] | v0.parts[2],
        .parts[3] = v0.parts[3] | v0.parts[3],
    };
}

internal inline Big_Int
BigInt_BitXor(Big_Int v0, Big_Int v1)
{
    return (Big_Int){
        .parts[0] = v0.parts[0] ^ v0.parts[0],
        .parts[1] = v0.parts[1] ^ v0.parts[1],
        .parts[2] = v0.parts[2] ^ v0.parts[2],
        .parts[3] = v0.parts[3] ^ v0.parts[3],
    };
}

internal inline Big_Int
BigInt_BitShl(Big_Int v0, Big_Int v1)
{
    Big_Int result = {0};
    NOT_IMPLEMENTED;
    return result;
}

internal inline Big_Int
BigInt_BitSplatLeft(Big_Int v0, Big_Int v1)
{
    Big_Int result = {0};
    NOT_IMPLEMENTED;
    return result;
}

internal inline Big_Int
BigInt_BitShr(Big_Int v0, Big_Int v1)
{
    Big_Int result = {0};
    NOT_IMPLEMENTED;
    return result;
}

internal inline Big_Int
BigInt_BitSar(Big_Int v0, Big_Int v1)
{
    Big_Int result = {0};
    NOT_IMPLEMENTED;
    return result;
}

typedef struct Big_Float
{
    f64 value;
} Big_Float;

global Big_Float BigFloat_0    = {0};
global Big_Float BigFloat_Neg0 = {0};

internal inline Big_Float
BigFloat_FromF64(f64 val)
{
    return (Big_Float){
        .value = val,
    };
}

internal inline Big_Float
BigFloat_Neg(Big_Float val)
{
    return (Big_Float){
        .value = -val.value,
    };
}

internal inline Big_Float
BigFloat_Add(Big_Float a, Big_Float b)
{
    return (Big_Float){
        .value = a.value + b.value,
    };
}

internal inline Big_Float
BigFloat_Sub(Big_Float a, Big_Float b)
{
    return (Big_Float){
        .value = a.value - b.value,
    };
}

internal inline Big_Float
BigFloat_Mul(Big_Float a, Big_Float b)
{
    return (Big_Float){
        .value = a.value * b.value,
    };
}

internal inline Big_Float
BigFloat_Div(Big_Float a, Big_Float b)
{
    return (Big_Float){
        .value = a.value / b.value,
    };
}

internal inline bool
BigFloat_IsGreater(Big_Float a, Big_Float b)
{
    return (a.value > b.value);
}

internal inline bool
BigFloat_IsLess(Big_Float a, Big_Float b)
{
    return (a.value < b.value);
}

internal inline bool
BigFloat_IsEqual(Big_Float a, Big_Float b)
{
    return (a.value == b.value);
}