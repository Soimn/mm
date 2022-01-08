typedef struct Big_Int
{
    bool _stub;
} Big_Int;

struct Big_Float
{
    bool _stub;
} Big_Float;

internal u64 BigInt_ToU64(Big_Int val);
internal u64 BigFloat_ToF64(Big_Int val);