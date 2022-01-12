typedef struct Big_Int
{
    bool _stub;
} Big_Int;

typedef struct Big_Float
{
    bool _stub;
} Big_Float;

internal u64 BigInt_ToU64(Big_Int val);
internal Big_Int BigInt_FromU64(u64 val);
internal Big_Int BigInt_FromBigFloat(Big_Float val, bool* did_truncate);
internal umm U64_EffectiveByteSize(u64 val);
internal umm I64_EffectiveByteSize(i64 val);
internal umm BigInt_EffectiveByteSize(Big_Int val);
internal u64 BigFloat_ToF64(Big_Float val);
internal umm BigFloat_EffectiveByteSize(Big_Float val);
internal Big_Float BigFloat_FromF64(f64 val);
internal Big_Float BigFloat_FromBigInt(Big_Int val);

internal u64
BigInt_ToU64(Big_Int val)
{
    u64 result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Int
BigInt_FromU64(u64 val)
{
    Big_Int result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Int
BigInt_FromBigFloat(Big_Float val, bool* did_truncate)
{
    Big_Int result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal umm
U64_EffectiveByteSize(u64 val)
{
    umm result;
    
    if      (val <= U8_MAX)  result = 1;
    else if (val <= U16_MAX) result = 2;
    else if (val <= U32_MAX) result = 4;
    else                     result = 8;
    
    return result;
}

internal umm
I64_EffectiveByteSize(i64 val)
{
    umm result;
    
    if      (val >= I8_MAX  && val <= U8_MAX)  result = 1;
    else if (val >= I16_MAX && val <= U16_MAX) result = 2;
    else if (val >= I32_MAX && val <= U32_MAX) result = 4;
    else                                       result = 8;
    
    return result;
}

internal umm
BigInt_EffectiveByteSize(Big_Int val)
{
    umm result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal umm
BigFloat_EffectiveByteSize(Big_Float val)
{
    umm result = 0;
    
#if 0
    // TODO: verify this
    u64 bits = val.data.floating;
    
    i64 exponent = (i64)(bits &  ((u64)0x7FF << (63 - 12))) - 1023;
    u64 mantissa =       bits & ~((u64)0xFFF << (63 - 12));
    
    umm mantissa_length = 0;
    for (umm scan = mantissa; mantissa != 0; mantissa <<= 1) ++mantissa_length;
    
    // NOTE: subnormal numbers have one bit less precision
    if      (ABS(exponent) <= 15  && mantissa_length <= 10 - (exponent == -15))  result = 2;
    else if (ABS(exponent) <= 127 && mantissa_length <= 23 - (exponent == -127)) result = 4;
    else                                                                         result = 8;
#endif
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Float
BigFloat_FromF64(f64 val)
{
    Big_Float result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Big_Float
BigFloat_FromBigInt(Big_Int val)
{
    Big_Float result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}