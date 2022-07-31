#include <immintrin.h>

typedef struct MM_Soft_Int
{
    union
    {
        MM_u64 p[4];
        
        struct
        {
            MM_u128 lo;
            MM_u128 hi;
        };
    };
} MM_Soft_Int;

typedef MM_u8 MM_Soft_Int_Status;

enum MM_SOFT_INT_STATUS
{
    MM_SoftIntStatus_None      = 0,
    MM_SoftIntStatus_Carry     = 1,
    MM_SoftIntStatus_Overflow  = 2,
    MM_SoftIntStatus_DivByZero = 4,
};

MM_Soft_Int
MM_SoftInt_FromU64(MM_u64 u64)
{
    return (MM_Soft_Int){ .p[0] = u64 };
}

MM_Soft_Int
MM_SoftInt_FromI64(MM_i64 i64)
{
    MM_u64 fill = (i64 < 0 ? MM_U64_MAX : 0);
    return (MM_Soft_Int){
        .p[0] = (MM_u64)i64,
        .p[1] = fill,
        .p[2] = fill,
        .p[3] = fill,
    };
}

MM_Soft_Int
MM_SoftInt_Add(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result;
    MM_u8 c1 = _addcarry_u64(0,  a.p[0], b.p[0], &result.p[0]);
    MM_u8 c2 = _addcarry_u64(c1, a.p[1], b.p[1], &result.p[1]);
    MM_u8 c3 = _addcarry_u64(c2, a.p[2], b.p[2], &result.p[2]);
    MM_u8 c4 = _addcarry_u64(c3, a.p[3], b.p[3], &result.p[3]);
    
    *status |= (!((a.p[3] ^ b.p[3]) & MM_I64_MAX) && ((a.p[3] ^ result.p[3]) & MM_I64_MAX) ? MM_SoftIntStatus_Overflow : 0);
    *status |= (c4 ? MM_SoftIntStatus_Carry : 0);
    
    return result;
}

MM_Soft_Int
MM_SoftInt_AddU64(MM_Soft_Int val, MM_u64 u64, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result;
    MM_u8 c1 = _addcarry_u64(0,  val.p[0], u64, &result.p[0]);
    MM_u8 c2 = _addcarry_u64(c1, val.p[1], 0,   &result.p[1]);
    MM_u8 c3 = _addcarry_u64(c2, val.p[2], 0,   &result.p[2]);
    MM_u8 c4 = _addcarry_u64(c3, val.p[3], 0,   &result.p[3]);
    
    *status |= ((~val.p[3] & result.p[3]) & MM_I64_MAX ? MM_SoftIntStatus_Overflow : 0);
    *status |= (c4 ? MM_SoftIntStatus_Carry : 0);
    
    return result;
}

MM_Soft_Int
MM_SoftInt_Sub(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result;
    
    MM_u8 b1 = _subborrow_u64(0,  a.p[0], b.p[0], &result.p[0]);
    MM_u8 b2 = _subborrow_u64(b1, a.p[1], b.p[1], &result.p[1]);
    MM_u8 b3 = _subborrow_u64(b2, a.p[2], b.p[2], &result.p[2]);
    MM_u8 b4 = _subborrow_u64(b3, a.p[3], b.p[3], &result.p[3]);
    
    *status |= (!((a.p[3] ^ ~b.p[3]) & MM_I64_MAX) && ((a.p[3] ^ result.p[3]) & MM_I64_MAX) ? MM_SoftIntStatus_Overflow : 0);
    *status |= (b4 ? MM_SoftIntStatus_Carry : 0);
    
    return result;
}

MM_Soft_Int
MM_SoftInt_SubU64(MM_Soft_Int val, MM_u64 u64, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result;
    MM_u8 b1 = _subborrow_u64(0,  val.p[0], u64, &result.p[0]);
    MM_u8 b2 = _subborrow_u64(b1, val.p[1], 0,   &result.p[1]);
    MM_u8 b3 = _subborrow_u64(b2, val.p[2], 0,   &result.p[2]);
    MM_u8 b4 = _subborrow_u64(b3, val.p[3], 0,   &result.p[3]);
    
    *status |= ((val.p[3] & ~result.p[3]) & MM_I64_MAX ? MM_SoftIntStatus_Overflow : 0);
    *status |= (b4 ? MM_SoftIntStatus_Carry : 0);
    
    return result;
}

// NOTE: mulmns from Hacker's Delight, modified to compute only the low-order 256 bits and whether the high-order bits are non-zero (overflow).
//       The code is based on experimentation using compiler explorer with clang 14 on O2 (https://godbolt.org/z/3YqP5KjGf, version D is used)

MM_Soft_Int
MM_SoftInt_Mul(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result = {0};
    MM_bool overflow   = MM_false;
    
    for (MM_umm i = 0; i < 4; ++i)
    {
        MM_u128 k = 0;
        for (MM_umm j = 0; i + j < 4; ++j)
        {
            MM_u128 t = a.p[j] * b.p[i] + result.p[i + j] + k;
            
            result.p[i + j] = (MM_u64)t;
            
            k = t >> 64;
        }
        
        overflow |= (k != 0);
        
        for (MM_umm j = 4 - i; j < 4; ++j) overflow |= (a.p[j] + b.p[i] != a.p[i]);
    }
    
    *status |= (overflow ? MM_SoftIntStatus_Carry | MM_SoftIntStatus_Overflow : 0);
    
    return result;
}

MM_Soft_Int
MM_SoftInt_MulU64(MM_Soft_Int val, MM_u64 u64, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result = {0};
    MM_bool overflow   = MM_false;
    
    MM_u128 k = 0;
    for (MM_umm j = 0; j < 4; ++j)
    {
        MM_u128 t = val.p[j] * u64 + result.p[j] + k;
        
        result.p[j] = (MM_u64)t;
        
        k = t >> 64;
    }
    
    overflow |= (k != 0);
    
    *status |= (overflow ? MM_SoftIntStatus_Carry | MM_SoftIntStatus_Overflow : 0);
    
    return result;
}

MM_Soft_Int
MM_SoftInt_OrU64(MM_Soft_Int val, MM_u64 u64)
{
    MM_Soft_Int result = val;
    result.p[0] |= u64;
    return result;
}

MM_Soft_Int
MM_SoftInt_AndU64(MM_Soft_Int val, MM_u64 u64)
{
    MM_Soft_Int result = { .p[0] = val.p[0] & u64 };
    return result;
}

MM_Soft_Int
MM_SoftInt_XorU64(MM_Soft_Int val, MM_u64 u64)
{
    MM_Soft_Int result = val;
    result.p[0] ^= u64;
    return result;
}

// NOTE: The bit shifting code is based on experimentation in compiler explorer with clang 14 on O2, trying to recreate the code generated for
//       builtin 128 bit arithmetic with 64 bit shifts. The following 256 bit procedures are based on the 64 bit variants.
//       link: https://godbolt.org/z/arvsK89P3

MM_Soft_Int
MM_SoftInt_Shl(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.p[0];
    
    MM_u128 hi = a.hi << shift_amount | a.lo >> (128 - shift_amount);
    MM_u128 lo = a.lo << shift_amount;
    
    if (shift_amount & 128)
    {
        hi = lo;
        lo = 0;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_ShlU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Shl(val, (MM_Soft_Int){ .p[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Shr(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.p[0];
    
    MM_u128 lo = a.lo >> shift_amount | a.hi << (128 - shift_amount);
    MM_u128 hi = a.hi >> shift_amount;
    
    if (shift_amount & 128)
    {
        lo = hi;
        hi= 0;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_ShrU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Shr(val, (MM_Soft_Int){ .p[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Sar(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.p[0];
    
    MM_u128 lo = a.lo >> shift_amount | a.hi << (128 - shift_amount);
    MM_u128 hi = (MM_i128)a.hi >> shift_amount;
    
    if (shift_amount & 128)
    {
        lo = hi;
        hi = (MM_i128)a.hi >> 127;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_SarU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Sar(val, (MM_Soft_Int){ .p[0] = u64 });
}