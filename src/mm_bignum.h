typedef struct Big_Int
{
    u64 value;
    i8 sign;
} Big_Int;

global Big_Int BigInt_0       = { .sign = 1, .value = 0 }; // NOTE: 0 counts as positive in this case
global Big_Int BigInt_U64_MAX = { .sign = 1, .value = U64_MAX };

internal inline Big_Int
BigInt_FromU64(u64 val)
{
    return (Big_Int){
        .value = val,
        .sign  = 1,
    };
}

internal inline u64
BigInt_ToU64(Big_Int val)
{
    ASSERT(val.sign == 1);
    
    return val.value;
}

internal inline Big_Int
BigInt_FromI64(i64 val)
{
    return (Big_Int){
        .value = (val < 0 ? -val : val),
        .sign  = (val < 0 ? -1   : 1),
    };
}

internal inline Big_Int
BigInt_Neg(Big_Int val)
{
    return (Big_Int){
        .value = val.value,
        .sign  = -val.sign,
    };
}

internal inline Big_Int
BigInt_Add(Big_Int a, Big_Int b)
{
    u64 result = (a.sign == 1 ? a.value : ~a.value + 1) + (b.sign == 1 ? b.value : ~b.value + 1);
    
    return (Big_Int){
        .sign  = ((result & ((u64)1 << 63)) != 0 ? -1 : 1),
        .value = result,
    };
}

internal inline Big_Int
BigInt_Sub(Big_Int a, Big_Int b)
{
    u64 result = (a.sign == 1 ? a.value : ~a.value + 1) + ~(b.sign == 1 ? b.value : ~b.value + 1) + 1;
    
    return (Big_Int){
        .sign  = ((result & ((u64)1 << 63)) != 0 ? -1 : 1),
        .value = result,
    };
}

internal inline Big_Int
BigInt_Mul(Big_Int a, Big_Int b)
{
    return (Big_Int){
        .sign  = a.sign  * b.sign,
        .value = a.value * b.value,
    };
}

internal inline Big_Int
BigInt_Div(Big_Int a, Big_Int b)
{
    return (Big_Int){
        .sign  = a.sign  * b.sign,
        .value = a.value / b.value,
    };
}

internal inline Big_Int
BigInt_Rem(Big_Int a, Big_Int b)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal inline bool
BigInt_IsGreater(Big_Int a, Big_Int b)
{
    return (a.sign == b.sign ? (a.value > b.value) ^ (a.sign == -1) : a.sign > b.sign);
}

internal inline bool
BigInt_IsLess(Big_Int a, Big_Int b)
{
    return (a.sign == b.sign ? (a.value < b.value) ^ (a.sign == -1) : a.sign < b.sign);
}

internal inline bool
BigInt_IsEqual(Big_Int a, Big_Int b)
{
    return (a.sign == b.sign && a.value == b.value);
}


internal inline umm
ChopUint(umm val, umm size)
{
    ASSERT(size > 0 && size <= 8 && ((size - 1) & size) == 0);
    
    u64 masks[] = {
        0xFF,
        0xFFFF,
        0xFFFFFFFF,
        0xFFFFFFFFFFFFFFFF,
    };
    
    return val & masks[size - 1];
}

internal inline umm
ChopInt(umm val, umm size)
{
    ASSERT(size > 0 && size <= 8 && ((size - 1) & size) == 0);
    
    u64 masks[] = {
        0xFF,
        0xFFFF,
        0xFFFFFFFF,
        0xFFFFFFFFFFFFFFFF,
    };
    
    return val & masks[size - 1];
}

internal inline imm
SignExtend(imm val, umm src_size)
{
    imm result;
    if      (src_size == 1) result = (i8)val;
    else if (src_size == 2) result = (i16)val;
    else if (src_size == 4) result = (i32)val;
    else if (src_size == 8) result = val;
    else INVALID_CODE_PATH;
    
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

internal inline f32
AbsF32(f32 n)
{
    union
    {
        u32 i;
        f32 f;
    } bits = { .f = n };
    
    bits.i &= U32_MAX >> 1;
    
    return bits.f;
}

internal inline f64
AbsF64(f64 n)
{
    union
    {
        u64 i;
        f64 f;
    } bits = { .f = n };
    
    bits.i &= U64_MAX >> 1;
    
    return bits.f;
}