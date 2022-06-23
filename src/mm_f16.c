#define MM_F16_SIGNIFICAND_SIZE 10
#define MM_F64_SIGNIFICAND_SIZE 52
#define MM_F16_EXP_BIAS 15
#define MM_F64_EXP_BIAS 1023
#define MM_F16_SIGNIFICAND_MASK 0x3FF
#define MM_F64_SIGNIFICAND_MASK 0xFFFFFFFFFFFFF
#define MM_F16_EXP_SIZE 5
#define MM_F64_EXP_SIZE 11

typedef union MM_F16_Bits
{
    MM_f16 bits;
    
    struct
    {
        MM_u16 significand : 10;
        MM_u16 exponent : 5;
        MM_u16 sign : 1;
    };
} MM_F16_Bits;

typedef union MM_F64_Bits
{
    MM_u64 bits;
    MM_f64 f;
    
    struct
    {
        MM_u64 significand : 52;
        MM_u64 exponent : 11;
        MM_u64 sign : 1;
    };
} MM_F64_Bits;

// NOTE: It took me 4+ hours of understanding and adapting Fabien Giesen's code,
//       fortunately making me understand how float conversion works, but it was
//       only after the fact that I realized there are intrinsics for this...
//       The final version will probably use the intrinsics, but this might
//       be useful later, if I feel like going through the absolute hell that is
//       implementing 128-bit IEEE 754 floating point

// NOTE: https://gist.github.com/rygorous/2144712 used as reference
MM_API MM_f64
MM_F64_FromF16(MM_f16 float16)
{
    typedef union MM_F16_Bits
    {
        MM_f16 bits;
        
        struct
        {
            MM_u16 significand : 10;
            MM_u16 exponent : 5;
            MM_u16 sign : 1;
        };
    } MM_F16_Bits;
    
    typedef union MM_F64_Bits
    {
        MM_u64 bits;
        MM_f64 f;
        
        struct
        {
            MM_u64 significand : 52;
            MM_u64 exponent : 11;
            MM_u64 sign : 1;
        };
    } MM_F64_Bits;
    
    MM_F16_Bits f = { .bits = float16 };
    
    // NOTE: Signed 0 by default
    MM_F64_Bits bits = { .sign = f.sign };
    
    // NOTE: denormal or signed 0
    if (f.exponent == 0)
    {
        // NOTE: denormal
        if (f.significand != 0)
        {
            MM_u16 significand = f.significand;
            MM_imm exponent    = -14;
            
            // NOTE: denormal numbers are not normalized, normalize and keep track of change to exponent
            while ((significand & ((MM_u16)1 << MM_F16_SIGNIFICAND_SIZE)) == 0)
            {
                exponent     -= 1;
                significand <<= 1;
            }
            
            bits.exponent    = exponent + MM_F64_EXP_BIAS;
            bits.significand = (MM_u64)(significand & MM_F16_SIGNIFICAND_MASK) << (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE);
        }
    }
    // NOTE: signed inf or NaN
    else if (f.exponent == ((MM_umm)1 << MM_F16_EXP_SIZE) - 1)
    {
        // NOTE: significand == 0 => signed inf, significand != 0 => sNaN/qNaN
        //       NaN and qNaN are typically distinguished by the most significant bit
        //       in the significand (1 for sNaN), according to comments in the reference
        //       it is safe to truncate the lower bits of a NaNs significand by shifting
        //       it up to the "binary point" of a single precision float. I assume this also
        //       holds true for half to double.
        bits.exponent    = ((MM_umm)1 << MM_F64_EXP_SIZE) - 1;
        bits.significand = (MM_u64)f.significand << (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE);
    }
    else
    {
        bits.exponent    = ((MM_i64)f.exponent - MM_F16_EXP_BIAS) + MM_F64_EXP_BIAS;
        bits.significand = (MM_u64)f.significand << (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE);
    }
    
    return bits.f;
}

// NOTE: https://gist.github.com/rygorous/2144712 used as reference
MM_API MM_f16
MM_F64_ToF16(MM_f64 float64)
{
    MM_F64_Bits bits = { .f = float64 };
    
    // NOTE: Signed 0 by default
    MM_F16_Bits f = { .sign = (MM_u16)bits.sign };
    
    // NOTE: Signed inf and NaN
    if (bits.exponent == ((MM_umm)1 << MM_F64_EXP_SIZE) - 1)
    {
        // NOTE: NaN is converted to qNaN, signed inf is kept as is
        f.significand = (bits.significand == 0 ? 0 : 0x200);
        f.exponent    = ((MM_umm)1 << MM_F16_EXP_SIZE) - 1;
    }
    // NOTE: Normal double numbers
    else
    {
        MM_imm exponent = (bits.exponent - MM_F64_EXP_BIAS) + MM_F16_EXP_BIAS;
        
        // NOTE: exponent might be too large (float overflow), return signed inf if this is true
        if (exponent >= 2 * MM_F16_EXP_BIAS + 1)
        {
            f.significand = 0;
            f.exponent    = ((MM_umm)1 << MM_F16_EXP_SIZE) - 1;
        }
        // NOTE: exponent might be too small (float underflow), adjust if the normalized value is representable, else return 0
        else if (exponent <= 0)
        {
            // NOTE: check if the significand will be non zero by checking if the number of digits shifted out is less than the
            //       number of digits in the significand (this includes the implicit 1)
            MM_umm num_digits_shifted_out = (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE) + 1;
            if ((num_digits_shifted_out + (MM_umm)-exponent) <= MM_F64_SIGNIFICAND_SIZE + 1)
            {
                MM_u64 significand  = bits.significand | (MM_u64)1 << MM_F64_SIGNIFICAND_SIZE;
                MM_umm shift_amount = num_digits_shifted_out + (MM_umm)-exponent;
                
                f.significand = (MM_u16)(significand >> shift_amount);
                // NOTE: f.exponent is left as 0 to make the number denormal
                
                MM_u64 significand_shifted_out            = (MM_u16)(significand & (((MM_u64)1 << shift_amount) - 1));
                MM_u64 most_significant_shifted_out_digit = (MM_u64)1 << (shift_amount - 1);
                
                // NOTE: This is the same rounding procedure as the one for normal numbers below (the big comment, if not outdated).
                //       The gist is that if the most significant digit that was shifted out is set, the shifted out part is larger
                //       or equal to half the least significant digit in the f16 significand. If the shifted out part is larger
                //       (i.e. more bits that were shifted out are set) or the shifted out part consists only of that set bit and
                //       the f16 significand is odd, the f16 bits are incremented to round up to the nearest even. This increment
                //       might overflow from the significand into the exponent, turning the f16 from a denormal number to a normal
                //       number.
                if (significand_shifted_out > most_significant_shifted_out_digit || significand_shifted_out ==  most_significant_shifted_out_digit && (f.significand & 1) != 0)
                {
                    f.bits += 1;
                }
            }
        }
        else
        {
            f.significand = (MM_u16)(bits.significand >> (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE));
            f.exponent    = (MM_u16)exponent;
            
            // NOTE: If the shifted out bits have the most significant bit set and the remaining bits are either non zero,
            //       or the least significant bit that __was not__ shifted out (i.e. f.significand & 1) is set, then
            //       the bits of the result (f) is incremented as an unsigned 16 bit integer. This will round up to the
            //       nearest even if that value is representable as an f16 (which may involve overflowing the significand).
            //       Otherwise, if the value is not representable as an f16, the overflow of the significand will increment
            //       the exponent (which must be 0x1E, otherwise the value is representable) from 0x1E to the max value 0x1F,
            //       producing a signed inf. (What I mean by representable is that there is a non inf/NaN f16 value that is as
            //       close to the actual value as possible. Increasing the significand by 1 might not always increase the f16
            //       value by 1. This means the "nearest representable even" is kind of misleading, since the value is the
            //       nearest non inf/NaN value that is even, but it might not be a the closest multiple of 2)
            MM_u64 shifted_out_mask     = MM_F64_SIGNIFICAND_MASK >> (MM_F64_SIGNIFICAND_SIZE - MM_F16_SIGNIFICAND_SIZE);
            MM_u64 most_significant_bit = (shifted_out_mask >> 1) + 1;
            if ((bits.significand & shifted_out_mask) > most_significant_bit || bits.significand == most_significant_bit && (f.significand & 1) != 0)
            {
                f.bits += 1;
            }
        }
    }
    
    return f.bits;
}