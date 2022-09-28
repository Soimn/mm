#define MM_F16_SIGNIFICAND_BITS 10
#define MM_F16_EXPONENT_BITS 5
#define MM_F16_EXPONENT_BIAS 15

typedef struct MM_f16
{
    union
    {
        MM_u16 bits;
        
        struct
        {
            MM_u16 significand : MM_F16_SIGNIFICAND_BITS;
            MM_u16 exponent    : MM_F16_EXPONENT_BITS;
            MM_u16 sign        : 1;
        };
    };
} MM_f16;

MM_f64
MM_F16_ToF64(MM_f16 f16)
{
    MM_NOT_IMPLEMENTED;
    return 0;
}

MM_f16
MM_F16_FromF64(MM_f64 f64)
{
    MM_NOT_IMPLEMENTED;
    return (MM_f16){};
}