typedef struct MM_Soft_Int
{
    union
    {
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

typedef MM_u8 MM_Soft_Int_Status;
enum MM_SOFT_INT_STATUS
{
    MM_SoftIntStatus_CarryBit     = 1,
    MM_SoftIntStatus_OverflowBit  = 2,
    MM_SoftIntStatus_DivByZeroBit = 4,
};

MM_Soft_Int
MM_SoftInt_Add(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_AddU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_Sub(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_SubU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_Mul(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_MulU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_Div(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int* r, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}

MM_Soft_Int
MM_SoftInt_DivU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int* r, MM_Soft_Int_Status* status)
{
    MM_NOT_IMPLEMENTED;
    return (MM_Soft_Int){};
}