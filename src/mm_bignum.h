typedef enum BIGNUM_STATUS
{
    BigNumStatus_None = 0, // NOTE: Used for initialization
    
    BigNumStatus_Carry    = 0x1,
    BigNumStatus_Overflow = 0x2,
} BIGNUM_STATUS;

typedef struct Big_Int
{
    u64 pieces[4];
} Big_Int;

global Big_Int Big_0 = {0};

internal Big_Int
BigInt_FromU64(u64 val)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_ChopTo(Big_Int a, umm byte_size)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal u64
BigInt_ChopToU64(Big_Int a, umm byte_size)
{
    NOT_IMPLEMENTED;
    return 0;
}

internal Big_Int
BigInt_MulAddU64(Big_Int big, u64 mul, u64 add, BIGNUM_STATUS* status)
{
    NOT_IMPLEMENTED;
}

internal Big_Int
BigInt_Mul(Big_Int a, Big_Int b)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_MulU64(Big_Int a, u64 u)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_Div(Big_Int a, Big_Int b)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_DivU64(Big_Int a, u64 u)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_Neg(Big_Int a)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal bool
BigInt_IsZero(Big_Int a)
{
    NOT_IMPLEMENTED;
    return false;
}

internal bool
BigInt_IsLess(Big_Int a, Big_Int b)
{
    NOT_IMPLEMENTED;
    return false;
}

internal bool
BigInt_IsLessU64(Big_Int a, u64 u)
{
    NOT_IMPLEMENTED;
    return false;
}

typedef struct Big_Float
{
    u64 pieces[4];
} Big_Float;

internal Big_Float
BigFloat_FromScientificNotation(Big_Int integeral, Big_Int fractional, Big_Int exponent)
{
    NOT_IMPLEMENTED;
    return (Big_Float){0};
}

internal Big_Float
BigFloat_FromBits(Big_Int bits, umm count)
{
    NOT_IMPLEMENTED;
    return (Big_Float){0};
}