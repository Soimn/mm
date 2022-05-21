// NOTE: mul & mul256
// let x = 2^64
// let a = a1x + a0
// let b = b1x + b0
// a*b = (a1x + a0) * (b1x + b0) = a1b1x^2 + a1b0x + a0b1x + a0b0
// mul:    (a*b) mod x^2 = (a1b1x^2 + a1b0x + a0b1x + a0b0) mod x^2 = (a1b0x + a0b1x + a0b0) mod x^2
// mul256: (a*b) mod x^4 = (a1b1x^2 + a1b0x + a0b1x + a0b0) mod x^4 = a1b1x^2 + a1b0x + a0b1x + a0b0

typedef union X128
{
    u32 pieces[4];
    struct
    {
        u64 lo;
        u64 hi;
    };
} X128;

typedef X128 I128;
typedef X128 U128;

typedef union I256
{
    u64 pieces[4];
    struct
    {
        U128 lo;
        U128 hi;
    };
} I256;

#define U128_0 (U128){0}
#define I128_0 (I128){0}
#define I256_0 (I256){0}

#define U128_MAX (U128){ .hi = U64_MAX, .lo = U64_MAX }

#define I128_MIN (I256){ .hi = I64_MIN }
#define I256_MIN (I256){ .pieces[3] = I64_MIN }

#define I128_MAX (I128){ .hi = I64_MAX, .lo = U64_MAX }
#define I256_MAX (I256){ .pieces[3] = I64_MAX, .pieces[2] = U64_MAX, .pieces[1] = U64_MAX, .pieces[0] = U64_MAX }

internal X128
X128_Add(u8 carry_in, X128 a, X128 b, u8* carry_out)
{
    X128 result;
    *carry_out = _addcarry_u64(_addcarry_u64(carry_in, a.lo, b.lo, &result.lo), a.hi, b.hi, &result.hi);
    
    return result;
}

internal X128
X128_Sub(u8 borrow_in, X128 a, X128 b, u8* borrow_out)
{
    X128 result;
    *borrow_out = _subborrow_u64(_subborrow_u64(borrow_in, a.lo, b.lo, &result.lo), a.hi, b.hi, &result.hi);
    
    return result;
}

internal X128
X128_Neg(X128 n)
{
    X128 result;
    _subborrow_u64(_subborrow_u64(0, 0, n.lo, &result.lo), 0, n.hi, &result.hi);
    
    return result;
}

internal X128
X128_Mul(X128 a, X128 b)
{
    u64 a0 = a.lo;
    u64 a1 = a.hi;
    
    u64 b0 = b.lo;
    u64 b1 = b.hi;
    
    u64 c0;
    X128 result;
    result.lo = _umul128(a0, b0, &c0);
    result.hi = a1*b0 + a0*b1 + c0;
    
    return result;
}

internal X128
X128_BitAnd(X128 a, X128 b)
{
    return (X128){
        .hi = a.hi & b.hi,
        .lo = a.lo & b.lo,
    };
}

internal X128
X128_BitOr(X128 a, X128 b)
{
    return (X128){
        .hi = a.hi | b.hi,
        .lo = a.lo | b.lo,
    };
}

internal X128
X128_BitXor(X128 a, X128 b)
{
    return (X128){
        .hi = a.hi ^ b.hi,
        .lo = a.lo ^ b.lo,
    };
}

internal X128
X128_BitNot(X128 n)
{
    return (X128){
        .hi = ~n.hi,
        .lo = ~n.lo,
    };
}

internal X128
X128_BitShl(X128 a, umm b)
{
    umm shift = b & (128 - 1);
    
    if (shift < 64)
    {
        return (X128){
            .lo = a.lo << shift,
            .hi = a.hi << shift | a.lo >> (64 - shift),
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (X128){
            .lo = 0,
            .hi = a.lo << adj_shift,
        };
    }
}

internal X128
X128_BitShr(X128 a, umm b)
{
    umm shift = b & (128 - 1);
    
    if (shift < 64)
    {
        return (X128){
            .lo = a.lo >> shift | a.hi << (64 - shift),
            .hi = a.hi >> shift,
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (X128){
            .lo = a.hi >> adj_shift,
            .hi = 0,
        };
    }
}

internal X128
X128_BitSar(X128 a, umm b)
{
    umm shift = b & (128 - 1);
    
    if (shift < 64)
    {
        return (X128){
            .lo = a.lo >> shift | a.hi << (64 - shift),
            .hi = (i64)a.hi >> shift,
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (X128){
            .lo = a.hi >> adj_shift,
            .hi = (i64)a.hi >> 63,
        };
    }
}

internal bool
X128_IsEqual(X128 a, X128 b)
{
    return (a.hi == b.hi && a.lo == b.lo);
}

internal bool
X128_IsZero(X128 n)
{
    return (n.hi == 0 && n.lo == 0);
}

internal bool
X128_IsLess(X128 a, X128 b)
{
    return (X128_Sub(0, a, b, &(u8){0}).hi > I64_MAX);
}

internal bool
X128_IsGreater(X128 a, X128 b)
{
    return (X128_Sub(0, b, a, &(u8){0}).hi > I64_MAX);
}

internal u64
X128_ChopToU64(X128 n, umm byte_size)
{
    ASSERT(IS_POW_OF_2(byte_size) && byte_size <= 8);
    
    return (byte_size == 8 ? n.lo : n.lo & ~(U64_MAX >> (byte_size*8)));
}

internal I128
I128_FromI64(i64 n)
{
    return (I128){
        .lo = n,
        .hi = n >> 63
    };
}

internal I128 I128_Add(I128 a, I128 b)                                     { return X128_Add(0, a, b,&(u8){0});            }
internal I128 I128_Sub(I128 a, I128 b)                                     { return X128_Sub(0, a, b, &(u8){0});           }
internal I128 I128_AddCarry(u8 carry_in, I128 a, I128 b, u8* carry_out)    { return X128_Add(carry_in, a, b, carry_out);   }
internal I128 I128_SubBorrow(u8 borrow_in, I128 a, I128 b, u8* borrow_out) { return X128_Sub(borrow_in, a, b, borrow_out); }
internal I128 I128_Neg(I128 n)                                             { return X128_Neg(n);                           }
internal I128 I128_Mul(I128 a, I128 b)                                     { return X128_Mul(a, b);                        }
internal I128 I128_BitAnd(I128 a, I128 b)                                  { return X128_BitAnd(a, b);                     }
internal I128 I128_BitOr(I128 a, I128 b)                                   { return X128_BitOr(a, b);                      }
internal I128 I128_BitXor(I128 a, I128 b)                                  { return X128_BitXor(a, b);                     }
internal I128 I128_BitNot(I128 n)                                          { return X128_BitNot(n);                        }
internal I128 I128_BitShl(I128 a, I128 b)                                  { return X128_BitShl(a, b.lo);                  }
internal I128 I128_BitShr(I128 a, I128 b)                                  { return X128_BitShr(a, b.lo);                  }
internal I128 I128_BitSar(I128 a, I128 b)                                  { return X128_BitSar(a, b.lo);                  }
internal bool I128_IsEqual(I128 a, I128 b)                                 { return X128_IsEqual(a, b);                    }
internal bool I128_IsZero(I128 n)                                          { return X128_IsZero(n);                        }
internal bool I128_IsLess(I128 a, I128 b)                                  { return X128_IsLess(a, b);                     }
internal bool I128_IsGreater(I128 a, I128 b)                               { return X128_IsGreater(a, b);                  }
internal u64  I128_ChopToU64(I128 n, umm byte_size)                        { return X128_ChopToU64(n, byte_size);          }

internal U128
U128_FromU64(u64 n)
{
    return (U128){ .lo = n };
}

internal U128 U128_Add(U128 a, U128 b)                                     { return X128_Add(0, a, b,&(u8){0});            }
internal U128 U128_Sub(U128 a, U128 b)                                     { return X128_Sub(0, a, b, &(u8){0});           }
internal U128 U128_AddCarry(u8 carry_in, U128 a, U128 b, u8* carry_out)    { return X128_Add(carry_in, a, b, carry_out);   }
internal U128 U128_SubBorrow(u8 borrow_in, U128 a, U128 b, u8* borrow_out) { return X128_Sub(borrow_in, a, b, borrow_out); }
internal U128 U128_Neg(U128 n)                                             { return X128_Neg(n);                           }
internal U128 U128_Mul(U128 a, U128 b)                                     { return X128_Mul(a, b);                        }
internal U128 U128_BitAnd(U128 a, U128 b)                                  { return X128_BitAnd(a, b);                     }
internal U128 U128_BitOr(U128 a, U128 b)                                   { return X128_BitOr(a, b);                      }
internal U128 U128_BitXor(U128 a, U128 b)                                  { return X128_BitXor(a, b);                     }
internal U128 U128_BitNot(U128 n)                                          { return X128_BitNot(n);                        }
internal U128 U128_BitShl(U128 a, U128 b)                                  { return X128_BitShl(a, b.lo);                  }
internal U128 U128_BitShr(U128 a, U128 b)                                  { return X128_BitShr(a, b.lo);                  }
internal U128 U128_BitSar(U128 a, U128 b)                                  { return X128_BitSar(a, b.lo);                  }
internal bool U128_IsEqual(U128 a, U128 b)                                 { return X128_IsEqual(a, b);                    }
internal bool U128_IsZero(U128 n)                                          { return X128_IsZero(n);                        }
internal bool U128_IsLess(U128 a, U128 b)                                  { return X128_IsLess(a, b);                     }
internal bool U128_IsGreater(U128 a, U128 b)                               { return X128_IsGreater(a, b);                  }
internal u64  U128_ChopToU64(U128 n, umm byte_size)                        { return X128_ChopToU64(n, byte_size);          }

internal U128
U128_Mul256(U128 a, U128 b, U128* high)
{
    u64 a0 = a.lo;
    u64 a1 = a.hi;
    
    u64 b0 = b.lo;
    u64 b1 = b.hi;
    
    u64 c0;
    U128 result;
    result.lo = _umul128(a0, b0, &c0);
    
    u64 a1b0_hi, a0b1_hi;
    u64 a1b0_lo = _umul128(a1, b0, &a1b0_hi);
    u64 a0b1_lo = _umul128(a0, b1, &a0b1_hi);
    
    u64 r10, r11;
    u8 c10 = _addcarry_u64(0, c0, a1b0_lo, &r10);
    u8 c11 = _addcarry_u64(0, r10, a0b1_lo, &r11);
    
    result.hi = r11;
    
    u64 a1b1_hi;
    u64 a1b1_lo = _umul128(a1, b1, &a1b1_hi);
    
    u64 r20;
    u8 c20 = _addcarry_u64(c10, a1b0_hi, a0b1_hi, &r20);
    u8 c21 = _addcarry_u64(c11, r20, a1b1_lo, &r20);
    
    high->lo = r20;
    high->hi = a1b1_hi + c20 + c21;
    
    return result;
}

internal I256
I256_FromU64(u64 val)
{
    return (I256){
        .pieces[0] = val,
    };
}

internal I256
I256_Add(I256 a, I256 b, u8* carry)
{
    I256 result;
    u8 c0;
    
    result.lo = X128_Add(0,  a.lo, b.lo, &c0);
    result.hi = X128_Add(c0, a.hi, b.hi, carry);
    
    return result;
}

internal I256
I256_Sub(I256 a, I256 b, u8* borrow)
{
    I256 result;
    u8 b0;
    
    result.lo = X128_Sub(0,  a.lo, b.lo, &b0);
    result.hi = X128_Sub(b0, a.hi, b.hi, borrow);
    
    return result;
}

internal I256
I256_Neg(I256 n)
{
    I256 result;
    u8 b0;
    
    result.lo = X128_Sub(0,  (I128){0}, n.lo, &b0);
    result.hi = X128_Sub(b0, (I128){0}, n.hi, &(u8){0});
    
    return result;
}

internal I256
I256_Mul(I256 a, I256 b)
{
    I128 a0 = a.lo;
    I128 a1 = a.hi;
    
    I128 b0 = b.lo;
    I128 b1 = b.hi;
    
    I128 c0;
    I256 result;
    result.lo = U128_Mul256(a0, b0, &c0);
    result.hi = U128_Add(U128_Add(U128_Mul(a1, b0), U128_Mul(a0, b1)), c0);
    
    return result;
}

internal I256
I256_BitAnd(I256 a, I256 b)
{
    return (I256){
        .hi = U128_BitAnd(a.hi, b.hi),
        .lo = U128_BitAnd(a.lo, b.lo),
    };
}

internal I256
I256_BitOr(I256 a, I256 b)
{
    return (I256){
        .hi = U128_BitOr(a.hi, b.hi),
        .lo = U128_BitOr(a.lo, b.lo),
    };
}

internal I256
I256_BitXor(I256 a, I256 b)
{
    return (I256){
        .hi = U128_BitOr(a.hi, b.hi),
        .lo = U128_BitOr(a.lo, b.lo),
    };
}

internal I256
I256_BitNot(I256 n)
{
    return (I256){
        .hi = U128_BitNot(n.hi),
        .lo = U128_BitNot(n.lo),
    };
}

internal I256
I256_BitShl(I256 a, I256 b)
{
    umm shift = (u8)b.lo.lo;
    
    if (shift < 128)
    {
        return (I256){
            .lo = X128_BitShl(a.lo, shift),
            .hi = U128_BitOr(X128_BitShl(a.hi, shift), X128_BitShr(a.lo, 64 - shift)),
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (I256){
            .lo = {0},
            .hi = X128_BitShl(a.lo, adj_shift),
        };
    }
}

internal I256
I256_BitShr(I256 a, I256 b)
{
    umm shift = (u8)b.lo.lo;
    
    if (shift < 128)
    {
        return (I256){
            .lo = U128_BitOr(X128_BitShr(a.lo, shift), X128_BitShl(a.hi, 64 - shift)),
            .hi = X128_BitShr(a.hi, shift),
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (I256){
            .lo = X128_BitShr(a.hi, adj_shift),
            .hi = {0},
        };
    }
}

internal I256
I256_BitSar(I256 a, I256 b)
{
    umm shift = (u8)b.lo.lo;
    
    if (shift < 128)
    {
        return (I256){
            .lo = U128_BitOr(X128_BitShr(a.lo, shift), X128_BitShl(a.hi, 64 - shift)),
            .hi = X128_BitSar(a.hi, shift),
        };
    }
    else
    {
        umm adj_shift = shift - 64;
        
        return (I256){
            .lo = X128_BitShr(a.hi, adj_shift),
            .hi = (a.hi.hi > I64_MAX ? (U128){U64_MAX, U64_MAX} : (U128){0}),
        };
    }
}

internal bool
I256_IsEqual(I256 a, I256 b)
{
    return (U128_IsEqual(a.hi, b.hi) && U128_IsEqual(a.lo, b.lo));
}

internal bool
I256_IsZero(I256 n)
{
    return (U128_IsZero(n.hi) && U128_IsZero(n.lo));
}

internal bool
I256_IsLess(I256 a, I256 b)
{
    return (I256_Sub(a, b, &(u8){0}).hi.hi > I64_MAX);
}

internal bool
I256_IsGreater(I256 a, I256 b)
{
    return (I256_Sub(b, a, &(u8){0}).hi.hi > I64_MAX);
}

internal u64
I256_ChopToU64(I256 n, umm byte_size)
{
    ASSERT(IS_POW_OF_2(byte_size) && byte_size <= 8);
    
    return (byte_size == 8 ? n.lo.lo : n.lo.lo & ~(U64_MAX >> (byte_size*8)));
}

internal U128
I256_ChopToU128(I256 n)
{
    return n.lo;
}

internal bool
I256_AppendDigit(I256* n, u8 base, u8 digit)
{
    u64 n3 = n->pieces[3];
    u64 n2 = n->pieces[2];
    u64 n1 = n->pieces[1];
    u64 n0 = n->pieces[0];
    
    u8 b = base;
    
    u64 n0b_hi;
    n->pieces[0] = _umul128(n0, b, &n0b_hi) + digit;
    
    u64 n1b_hi;
    u64 n1b_lo = _umul128(n1, b, &n1b_hi);
    u8 c1 = _addcarry_u64(0, n0b_hi, n1b_lo, &n->pieces[1]);
    
    u64 n2b_hi;
    u64 n2b_lo = _umul128(n2, b, &n2b_hi);
    u8 c2 = _addcarry_u64(c1, n1b_hi, n2b_lo, &n->pieces[2]);
    
    return _addcarry_u64(c2, n3*b, n2b_hi, &n->pieces[3]);
}

#define F16_EXP_BIAS 15
#define F16_SIGNIFICAND_SIZE 10

typedef struct f16
{
    u16 bits;
    
    struct
    {
        u16 significand : F16_SIGNIFICAND_SIZE;
        u16 exp         : 5;
        u16 sign        : 1;
    };
} f16;

#define F32_EXP_BIAS 127
#define F32_SIGNIFICAND_SIZE 23

typedef union F32_Bits
{
    f32 f;
    u32 bits;
    
    struct
    {
        u32 significand : F32_SIGNIFICAND_SIZE;
        u32 exp         : 8;
        u32 sign        : 1;
    };
} F32_Bits;

#define F64_EXP_BIAS 1023
#define F64_SIGNIFICAND_SIZE 52

typedef union F64_Bits
{
    f64 f;
    u64 bits;
    
    struct
    {
        u64 significand : F64_SIGNIFICAND_SIZE;
        u64 exp         : 11;
        u64 sign        : 1;
    };
} F64_Bits;

internal f16
F16_FromBits(u16 bits)
{
    return (f16){ .bits = bits };
}

internal f64
F64_FromF16(f16 f)
{
    F64_Bits bits;
    
    bits.significand = (u64)f.significand << (F64_SIGNIFICAND_SIZE - F16_SIGNIFICAND_SIZE);
    bits.exp         = ((i64)f.exp - F16_EXP_BIAS) + F64_EXP_BIAS;
    bits.sign        = f.sign;
    
    // TODO: special: subnormal, inf, nan
    NOT_IMPLEMENTED;
}

internal f16
F64_ToF16(f64 f)
{
    F64_Bits bits = { .f = f };
    NOT_IMPLEMENTED;
}

internal f64
F64_FromParts(I256 integeral, I256 fractional, I256 exponent)
{
    NOT_IMPLEMENTED;
}