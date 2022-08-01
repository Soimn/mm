typedef struct MM_Soft_Int
{
    union
    {
        MM_u32 ph[8];
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

MM_Soft_Int
MM_SoftInt_Neg(MM_Soft_Int val, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result;
    
    MM_u8 b1 = _subborrow_u64(0,  0, val.p[0], &result.p[0]);
    MM_u8 b2 = _subborrow_u64(b1, 0, val.p[1], &result.p[1]);
    MM_u8 b3 = _subborrow_u64(b2, 0, val.p[2], &result.p[2]);
    MM_u8 b4 = _subborrow_u64(b3, 0, val.p[3], &result.p[3]);
    (void)b4;
    
    *status |= (!(~val.p[3] & MM_I64_MAX) && ((val.p[3] ^ result.p[3]) & MM_I64_MAX) ? MM_SoftIntStatus_Overflow : 0);
    *status |= (val.lo == 0 && val.hi == 0 ? 0 : MM_SoftIntStatus_Carry);
    
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

// NOTE: Based on divmnu64 from Hacker's Delight
MM_Soft_Int
MM_SoftInt_DivU32(MM_Soft_Int val, MM_u32 u32, MM_u32* remainder, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result = {0};
    
    if (u32 == 0) *status |= MM_SoftIntStatus_DivByZero;
    else
    {
        MM_u64 k = 0;
        for (MM_imm i = 8; i >= 0; --i)
        {
            MM_u32 dividend_digits = k << 32 | val.p[i];
            
            result.p[i] = dividend_digits / u32;
            k           = dividend_digits % u32;
        }
        
        *remainder = (MM_u32)k;
    }
    
    return result;
}

// NOTE: Based on divmnu64 from Hacker's Delight
MM_Soft_Int
MM_SoftInt_Div(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int* remainder, MM_Soft_Int_Status* status)
{
    MM_ZeroStruct(remainder);
    
    // NOTE: An attempt at computing the number of digits without introducing way too many branches
    MM_umm n;
    {
        MM_u32 nn = 1;
        nn |= (b.p[3] != 0) << 4;
        nn |= (b.p[2] != 0) << 3;
        nn |= (b.p[1] != 0) << 2;
        
        n = __builtin_clzll(nn);
        n = 2*(32 - n);
        n -= (b.ph[n - 1] == 0);
    }
    
    if (n == 1) return MM_SoftInt_DivU32(a, b.p[0], (MM_u32*)&remainder->p[0], status);
    else
    {
        MM_Soft_Int result = {0};
        
        // NOTE: Shift a and b up by the amount of leading zeros in b's leading digit.
        //       a is renamed to u, b is renamed to v (to fit the naming in Hacker's Delight).
        MM_umm shift_amount = __builtin_clzll(b.p[n - 1]);
        
        MM_u32 u[9];
        {
            u[0] = a.ph[0] << shift_amount;
            for (MM_umm i = 1; i < 8; ++i)
            {
                u[i] = a.ph[i] << shift_amount | (MM_u64)a.ph[i - 1] >> (32 - shift_amount);
            }
            u[8] = a.ph[7] >> (32 - shift_amount);
        }
        
        MM_u32 v[8];
        {
            v[0] = b.ph[0] << shift_amount;
            for (MM_umm i = 1; i < 8; ++i)
            {
                v[i] = b.ph[i] << shift_amount | (MM_u64)b.ph[i - 1] >> (32 - shift_amount);
            }
        }
        
        // NOTE: For each leading zero digit in v and v's leading non-zero digit, compute the
        //       division of u's j and j-1 th digit on v's leading digit. This is an estimate for
        //       the quotient, and is at most 2 off. The quotient is then refined.
        for (MM_imm j = 8 - n; j >= 0; --j)
        {
            MM_u64 dividend_digits = (MM_u64)u[j + n] << 32 | u[j + n - 1];
            MM_u64 qhat = dividend_digits / v[n - 1];
            MM_u64 rhat = dividend_digits % v[n - 1];
            
            // NOTE: Eliminate all cases where the quotient is 2 too high, and most where it is 1 too high
            do
            {
                if (qhat > MM_U32_MAX || qhat * v[n - 2] > (rhat << 32 | u[j + n - 2]))
                {
                    qhat -= 1;
                    rhat += v[n - 1];
                }
            } while (rhat < ((MM_u64)1 << 32));
            
            // NOTE: Multiply v by qhat and subtract it from u
            MM_i64 t = 0;
            {
                MM_i64 k = 0;
                for (MM_umm i = 0; i < n; ++i)
                {
                    MM_u64 p = qhat * v[i];
                    t = u[i + j] - k - (MM_u32)p;
                    
                    u[i + j] = (MM_u32)t;
                    k        = (p >> 32) - (t >> 32);
                    // NOTE: I think "- (t >> 32)" is another way of saying +1 if t is negative (in other words, borrow 1 from the next digit)
                }
                
                t        = u[j + n] - k;
                u[j + n] = (MM_u32)t;
                
                result.ph[j] = (MM_u32)qhat;
            }
            
            // NOTE: Decrement quotient and add v to u if u ended up negative after subtracting qhat*v.
            //       u is negative iff t is negative.
            if (t < 0)
            {
                result.ph[j] -= 1;
                
                MM_u64 k = 0;
                for (MM_umm i = 0; i < n; ++i)
                {
                    t = (MM_u64)u[i + j] + v[i] + k;
                    
                    u[i + j] = t;
                    k        = t >> 32;
                }
                
                u[j + n] += k;
            }
            
        }
        
        // NOTE: assign u shifted down by the normalization shift amount to the remainder
        for (MM_umm i = 0; i < n - 1; ++i)
        {
            remainder->ph[i] = (u[i] >> shift_amount) | ((MM_u64)u[i + 1] << (32 - shift_amount));
        }
        
        remainder->ph[n - 1] = u[n - 1] >> shift_amount;
        
        return result;
    }
}

MM_Soft_Int
MM_SoftInt_Not(MM_Soft_Int val)
{
    return (MM_Soft_Int){
        .lo = ~val.lo,
        .hi = ~val.hi,
    };
}

MM_Soft_Int
MM_SoftInt_Or(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo | b.lo,
        .hi = a.hi | b.hi,
    };
}

MM_Soft_Int
MM_SoftInt_OrU64(MM_Soft_Int val, MM_u64 u64)
{
    MM_Soft_Int result = val;
    result.p[0] |= u64;
    return result;
}

MM_Soft_Int
MM_SoftInt_And(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo & b.lo,
        .hi = a.hi & b.hi,
    };
}

MM_Soft_Int
MM_SoftInt_AndU64(MM_Soft_Int val, MM_u64 u64)
{
    return (MM_Soft_Int){ .p[0] = val.p[0] & u64 };
}

MM_Soft_Int
MM_SoftInt_Xor(MM_Soft_Int a, MM_Soft_Int b)
{
    return (MM_Soft_Int){
        .lo = a.lo ^ b.lo,
        .hi = a.hi ^ b.hi,
    };
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
MM_SoftInt_Spl(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.p[0];
    
    MM_u128 fill = (a.p[0] & 1 ? MM_U128_MAX : 0);
    
    MM_u128 hi = a.hi << shift_amount | a.lo >> (128 - shift_amount);
    MM_u128 lo = a.lo << shift_amount | fill >> (128 - shift_amount);
    
    if (shift_amount & 128)
    {
        hi = lo;
        lo = fill;
    }
    
    return (MM_Soft_Int){ .hi = hi, .lo = lo };
}

MM_Soft_Int
MM_SoftInt_SplU64(MM_Soft_Int val, MM_u64 u64)
{
    return MM_SoftInt_Spl(val, (MM_Soft_Int){ .p[0] = u64 });
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

MM_bool
MM_SoftInt_IsEqual(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi == b.hi && a.lo == b.lo);
}

MM_bool
MM_SoftInt_IsEqualU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo == u64);
}

MM_bool
MM_SoftInt_IsLess(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo));
}

MM_bool
MM_SoftInt_IsLessU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo < u64);
}

MM_bool
MM_SoftInt_IsLessEq(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi < b.hi || (a.hi == b.hi && a.lo <= b.lo));
}

MM_bool
MM_SoftInt_IsLessEqU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi == 0 && val.lo <= u64);
}

MM_bool
MM_SoftInt_IsGreater(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi >= b.hi && (a.hi != b.hi || a.lo > b.lo));
}

MM_bool
MM_SoftInt_IsGreaterU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi != 0 || val.lo > u64);
}

MM_bool
MM_SoftInt_IsGreaterEq(MM_Soft_Int a, MM_Soft_Int b)
{
    return (a.hi >= b.hi && (a.hi != b.hi || a.lo >= b.lo));
}

MM_bool
MM_SoftInt_IsGreaterEqU64(MM_Soft_Int val, MM_u64 u64)
{
    return (val.hi != 0 || val.lo >= u64);
}