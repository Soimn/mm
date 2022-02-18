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

typedef struct Big_Float
{
    f64 value;
} Big_Float;

global Big_Float BigFloat_0 = {0};

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
