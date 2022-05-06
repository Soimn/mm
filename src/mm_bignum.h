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

internal Big_Int
BigInt_FromU64(u64 val)
{
    NOT_IMPLEMENTED;
    return (Big_Int){0};
}

internal Big_Int
BigInt_MulAddU64(Big_Int big, u64 mul, u64 add, BIGNUM_STATUS* status)
{
    NOT_IMPLEMENTED;
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