typedef struct MM_Soft_Int
{
    union
    {
        MM_u64 eh[8];
        MM_u64 e[4];
        struct
        {
            MM_u128 lo;
            MM_u128 hi;
        };
    };
} MM_Soft_Int;

inline MM_Soft_Int
MM_SoftInt_FromU64(MM_u64 n)
{
    return (MM_Soft_Int){ .e[0] = n };
}

inline MM_u64
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

extern unsigned char _addcarry_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*);
extern unsigned char _subborrow_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64*);

inline MM_Soft_Int
MM_SoftInt_Neg(MM_Soft_Int n)
{
    MM_Soft_Int r;
    
    unsigned char c1 = _addcarry_u64(1,  ~n.e[0], 0, &r.e[0]);
    unsigned char c2 = _addcarry_u64(c1, ~n.e[1], 0, &r.e[1]);
    unsigned char c3 = _addcarry_u64(c2, ~n.e[2], 0, &r.e[2]);
    unsigned char c4 = _addcarry_u64(c3, ~n.e[3], 0, &r.e[3]);
    (void)c4;
    
    return r;
}

inline MM_bool
MM_SoftInt_IsNeg(MM_Soft_Int n)
{
    return ((MM_i64)n.e[3] < 0);
}

inline MM_bool
MM_SoftInt_IsZero(MM_Soft_Int n)
{
    return (n.lo == 0 && n.hi == 0);
}

inline MM_Soft_Int
MM_SoftInt_AddU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int r;
    
    unsigned char c1 = _addcarry_u64(0,  a.e[0], b, &r.e[0]);
    unsigned char c2 = _addcarry_u64(c1, a.e[1], 0, &r.e[1]);
    unsigned char c3 = _addcarry_u64(c2, a.e[2], 0, &r.e[2]);
    unsigned char c4 = _addcarry_u64(c3, a.e[3], 0, &r.e[3]);
    
    *status |= (c4 ? MM_SoftIntStatus_CarryBit : 0);
    *status |= ((MM_i64)a.e[3] >= 0 && (MM_i64)r.e[3] < 0 ? MM_SoftIntStatus_OverflowBit : 0);
    
    return r;
}

inline MM_Soft_Int
MM_SoftInt_Add(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int r;
    
    unsigned char c1 = _addcarry_u64(0,  a.e[0], b.e[0], &r.e[0]);
    unsigned char c2 = _addcarry_u64(c1, a.e[1], b.e[1], &r.e[1]);
    unsigned char c3 = _addcarry_u64(c2, a.e[2], b.e[2], &r.e[2]);
    unsigned char c4 = _addcarry_u64(c3, a.e[3], b.e[3], &r.e[3]);
    
    *status |= (c4 ? MM_SoftIntStatus_CarryBit : 0);
    *status |= ((MM_i64)(a.e[3] ^ b.e[3]) >= 0 && (MM_i64)(a.e[3] ^ r.e[3]) < 0 ? MM_SoftIntStatus_OverflowBit : 0);
    
    return r;
}

inline MM_Soft_Int
MM_SoftInt_SubU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_Soft_Int r;
    
    unsigned char b1 = _subborrow_u64(0,  a.e[0], b, &r.e[0]);
    unsigned char b2 = _subborrow_u64(b1, a.e[1], 0, &r.e[1]);
    unsigned char b3 = _subborrow_u64(b2, a.e[2], 0, &r.e[2]);
    unsigned char b4 = _subborrow_u64(b3, a.e[3], 0, &r.e[3]);
    
    *status |= (b4 ? MM_SoftIntStatus_CarryBit : 0);
    *status |= ((MM_i64)(a.e[3] ^ 0) < 0 && (MM_i64)(0 ^ r.e[3]) >= 0 ? MM_SoftIntStatus_OverflowBit : 0);
    
    return r;
}

inline MM_Soft_Int
MM_SoftInt_Sub(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    
    MM_Soft_Int r;
    
    unsigned char b1 = _subborrow_u64(0,  a.e[0], b.e[0], &r.e[0]);
    unsigned char b2 = _subborrow_u64(b1, a.e[1], b.e[1], &r.e[1]);
    unsigned char b3 = _subborrow_u64(b2, a.e[2], b.e[2], &r.e[2]);
    unsigned char b4 = _subborrow_u64(b3, a.e[3], b.e[3], &r.e[3]);
    
    *status |= (b4 ? MM_SoftIntStatus_CarryBit : 0);
    *status |= ((MM_i64)(a.e[3] ^ b.e[3]) < 0 && (MM_i64)(b.e[3] ^ r.e[3]) >= 0 ? MM_SoftIntStatus_OverflowBit : 0);
    
    return r;
}

inline MM_Soft_Int
MM_SoftInt_MulU64(MM_Soft_Int a, MM_u64 b, MM_Soft_Int_Status* status)
{
    MM_bool should_flip = MM_false;
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a);
        should_flip = MM_true;
    }
    
    MM_u128 a0b0 = (MM_u128)a.e[0] * b;
    MM_u128 a1b0 = (MM_u128)a.e[1] * b;
    MM_u128 a2b0 = (MM_u128)a.e[2] * b;
    MM_u128 a3b0 = (MM_u128)a.e[3] * b;
    
    MM_u128 r0 = a0b0 >> 64;
    MM_u128 r1 = a1b0 + (r0 >> 64);
    MM_u128 r2 = a2b0 + (r1 >> 64);
    MM_u128 r3 = a3b0 + (r2 >> 64);
    
    MM_Soft_Int r = {
        .e[0] = (MM_u64)r0,
        .e[1] = (MM_u64)r1,
        .e[2] = (MM_u64)r2,
        .e[3] = (MM_u64)r3,
    };
    
    if ((r3 >> 64) != 0)
    {
        *status |= MM_SoftIntStatus_CarryBit | MM_SoftIntStatus_OverflowBit;
    }
    else if (r.e[3] > ((MM_u64)1 << 63) && should_flip)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    if (should_flip) r = MM_SoftInt_Neg(r);
    
    return r;
}

inline MM_Soft_Int
MM_SoftInt_Mul(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int_Status* status)
{
    MM_bool should_flip = MM_false;
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a);
        should_flip = MM_true;
    }
    
    if (MM_SoftInt_IsNeg(b))
    {
        b           = MM_SoftInt_Neg(b);
        should_flip = !should_flip;
    }
    
    MM_u128 a0b0 = (MM_u128)a.e[0] * b.e[0];
    MM_u128 a1b0 = (MM_u128)a.e[1] * b.e[0];
    MM_u128 a2b0 = (MM_u128)a.e[2] * b.e[0];
    MM_u128 a3b0 = (MM_u128)a.e[3] * b.e[0];
    
    MM_u128 a0b1 = (MM_u128)a.e[0] * b.e[1];
    MM_u128 a1b1 = (MM_u128)a.e[1] * b.e[1];
    MM_u128 a2b1 = (MM_u128)a.e[2] * b.e[1];
    MM_u128 a3b1 = (MM_u128)a.e[3] * b.e[1];
    
    MM_u128 a0b2 = (MM_u128)a.e[0] * b.e[2];
    MM_u128 a1b2 = (MM_u128)a.e[1] * b.e[2];
    MM_u128 a2b2 = (MM_u128)a.e[2] * b.e[2];
    MM_u128 a3b2 = (MM_u128)a.e[3] * b.e[2];
    
    MM_u128 a0b3 = (MM_u128)a.e[0] * b.e[3];
    MM_u128 a1b3 = (MM_u128)a.e[1] * b.e[3];
    MM_u128 a2b3 = (MM_u128)a.e[2] * b.e[3];
    MM_u128 a3b3 = (MM_u128)a.e[3] * b.e[3];
    
#define MM_LO(X) ((X) & MM_U64_MAX)
#define MM_HI(X) ((X) >> 64)
    
    MM_u128 r0_lo = MM_LO(a0b0);
    MM_u128 r0_hi = MM_HI(a0b0);
    MM_u128 r1_lo = r0_hi + MM_LO(a1b0) + MM_LO(a0b1);
    MM_u128 r1_hi = MM_HI(r1_lo) + MM_HI(a1b0) + MM_HI(a0b1);
    MM_u128 r2_lo = r1_hi + MM_LO(a2b0) + MM_LO(a1b1) + MM_LO(a0b2);
    MM_u128 r2_hi = MM_HI(r2_lo) + MM_HI(a2b0) + MM_HI(a1b1) + MM_HI(a0b2);
    MM_u128 r3_lo = r2_hi + MM_LO(a3b0) + MM_LO(a2b1) + MM_LO(a1b2) + MM_LO(a0b3);
    MM_u128 r3_hi = MM_HI(r3_lo) + MM_HI(a3b0) + MM_HI(a2b1) + MM_HI(a1b2) + MM_HI(a0b3);
    
#undef MM_LO
#undef MM_HI
    
    MM_Soft_Int r = {
        .e[0] = (MM_u64)r0_lo,
        .e[1] = (MM_u64)r1_lo,
        .e[2] = (MM_u64)r2_lo,
        .e[3] = (MM_u64)r3_lo,
    };
    
    if (r3_hi != 0 || a3b3 != 0 || a3b2 != 0 || a3b1 != 0 || a2b3 != 0 || a2b2 != 0 || a1b3 != 0)
    {
        *status |= MM_SoftIntStatus_CarryBit | MM_SoftIntStatus_OverflowBit;
    }
    else if (r.e[3] > ((MM_u64)1 << 63) && should_flip)
    {
        *status |= MM_SoftIntStatus_OverflowBit;
    }
    
    if (should_flip) r = MM_SoftInt_Neg(r);
    
    return r;
}
// NOTE: Based on divmnu64 from Hacker's Delight
MM_Soft_Int
MM_SoftInt_DivU32(MM_Soft_Int val, MM_u32 u32, MM_u32* remainder, MM_Soft_Int_Status* status)
{
    MM_Soft_Int result = {0};
    
    MM_bool should_flip = MM_false;
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a);
        should_flip = MM_true;
    }
    
    if (u32 == 0) *status |= MM_SoftIntStatus_DivByZeroBit;
    else
    {
        MM_u64 k = 0;
        for (MM_imm i = 8; i >= 0; --i)
        {
            MM_u32 dividend_digits = k << 32 | val.e[i];
            
            result.e[i] = dividend_digits / u32;
            k           = dividend_digits % u32;
        }
        
        *remainder = (MM_u32)k;
    }
    
    // TODO: update flags
    MM_NOT_IMPLEMENTED;
    
    if (should_flip) result = MM_SoftInt_Neg(result);
    
    return result;
}

// NOTE: Based on divmnu64 from Hacker's Delight
MM_Soft_Int
MM_SoftInt_Div(MM_Soft_Int a, MM_Soft_Int b, MM_Soft_Int* remainder, MM_Soft_Int_Status* status)
{
    MM_ZeroStruct(remainder);
    
    MM_bool should_flip = MM_false;
    if (MM_SoftInt_IsNeg(a))
    {
        a           = MM_SoftInt_Neg(a);
        should_flip = MM_true;
    }
    
    if (MM_SoftInt_IsNeg(b))
    {
        b           = MM_SoftInt_Neg(b);
        should_flip = !should_flip;
    }
    
    // NOTE: An attempt at computing the number of digits without introducing way too many branches
    MM_umm n;
    {
        MM_u32 nn = 1;
        nn |= (b.e[3] != 0) << 4;
        nn |= (b.e[2] != 0) << 3;
        nn |= (b.e[1] != 0) << 2;
        
        n = __builtin_clzll(nn);
        n = 2*(32 - n);
        n -= (b.eh[n - 1] == 0);
    }
    
    MM_Soft_Int result = {0};
    
    if (n == 1) result = MM_SoftInt_DivU32(a, b.e[0], (MM_u32*)&remainder->e[0], status);
    else
    {
        // NOTE: Shift a and b up by the amount of leading zeros in b's leading digit.
        //       a is renamed to u, b is renamed to v (to fit the naming in Hacker's Delight).
        MM_umm shift_amount = __builtin_clzll(b.e[n - 1]);
        
        MM_u32 u[9];
        {
            u[0] = a.eh[0] << shift_amount;
            for (MM_umm i = 1; i < 8; ++i)
            {
                u[i] = a.eh[i] << shift_amount | (MM_u64)a.eh[i - 1] >> (32 - shift_amount);
            }
            u[8] = a.eh[7] >> (32 - shift_amount);
        }
        
        MM_u32 v[8];
        {
            v[0] = b.eh[0] << shift_amount;
            for (MM_umm i = 1; i < 8; ++i)
            {
                v[i] = b.eh[i] << shift_amount | (MM_u64)b.eh[i - 1] >> (32 - shift_amount);
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
                
                result.eh[j] = (MM_u32)qhat;
            }
            
            // NOTE: Decrement quotient and add v to u if u ended up negative after subtracting qhat*v.
            //       u is negative iff t is negative.
            if (t < 0)
            {
                result.eh[j] -= 1;
                
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
            remainder->eh[i] = (u[i] >> shift_amount) | ((MM_u64)u[i + 1] << (32 - shift_amount));
        }
        
        remainder->eh[n - 1] = u[n - 1] >> shift_amount;
    }
    
    // TODO: update flags
    MM_NOT_IMPLEMENTED;
    
    if (should_flip) result = MM_SoftInt_Neg(result);
    
    return result;
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
    result.e[0] |= u64;
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
    return (MM_Soft_Int){ .e[0] = val.e[0] & u64 };
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
    return (MM_Soft_Int){
        .lo = val.lo ^ u64,
        .hi = val.hi ^ 0,
    };
}

// NOTE: The bit shifting code is based on experimentation in compiler explorer with clang 14 on O2, trying to recreate the code generated for
//       builtin 128 bit arithmetic with 64 bit shifts. The following 256 bit procedures are based on the 64 bit variants.
//       link: https://godbolt.org/z/arvsK89P3

MM_Soft_Int
MM_SoftInt_Shl(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
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
    return MM_SoftInt_Shl(val, (MM_Soft_Int){ .e[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Shr(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
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
    return MM_SoftInt_Shr(val, (MM_Soft_Int){ .e[0] = u64 });
}

MM_Soft_Int
MM_SoftInt_Sar(MM_Soft_Int a, MM_Soft_Int b)
{
    MM_umm shift_amount = b.e[0];
    
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
    return MM_SoftInt_Sar(val, (MM_Soft_Int){ .e[0] = u64 });
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