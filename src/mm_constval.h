typedef struct Range
{
    Big_Int start;
    Big_Int end;
} Range;

typedef union Const_Val
{
    Big_Int big_int;
    Big_Float big_float;
    u64 uint64;
    i64 int64;
    f64 float64;
    f32 float32;
    bool boolean;
    Interned_String string;
    Range range;
} Const_Val;

internal bool
ConstVal_CoerceUntypedToType(Type_ID src, Type_ID dst, Const_Val val, Const_Val* result)
{
    bool encountered_errors = false;
    
    if (src == Type_UntypedInt)
    {
        NOT_IMPLEMENTED;
        //Type_Any,
        
        if (Type_IsBoolean(dst))
        {
            result->boolean = !BigInt_IsEqual(val.big_int, BigInt_0);
        }
        
        else if (Type_IsIntegral(dst) || dst == Type_Byte)
        {
            if (Type_IsSignedInteger(dst))
            {
                u64 max = ~(u64)0 >> (65 - 8*Type_Sizeof(dst));
                i64 min = -(i64)((u64)1 << (8*Type_Sizeof(dst) - 1));
                
                if (BigInt_IsGreater(val.big_int, BigInt_FromU64(max)))
                {
                    //// ERROR: untyped integer is too large
                    encountered_errors = true;
                }
                
                else if (BigInt_IsLess(val.big_int, BigInt_FromI64(min)))
                {
                    //// ERROR: untyped integer is too small
                    encountered_errors = true;
                }
                
                else
                {
                    result->int64 = BigInt_ToI64(val.big_int) & BYTE_MASK(Type_Sizeof(dst));
                }
            }
            
            else
            {
                if (BigInt_IsLess(val.big_int, BigInt_0))
                {
                    //// ERROR: untyped integer is negative
                    encountered_errors = true;
                }
                
                else if (BigInt_IsGreater(val.big_int, BigInt_FromU64(BYTE_MASK(Type_Sizeof(dst)))))
                {
                    //// ERROR: untyped integer is too large
                    encountered_errors = true;
                }
                
                else
                {
                    result->uint64 = BigInt_ToU64(val.big_int) & BYTE_MASK(Type_Sizeof(dst));
                }
            }
        }
        
        else if (Type_IsFloating(dst))
        {
            if (dst == Type_F64)
            {
                if (BigInt_IsGreater(val.big_int, BigInt_FromU64((u64)1 << 54)))
                {
                    //// ERROR: untyped integer is too large to fit into f64
                    encountered_errors = true;
                }
                
                else if (BigInt_IsLess(val.big_int, BigInt_FromU64((u64)1 << 54)))
                {
                    //// ERROR: untyped integer is too small to fit into f64
                    encountered_errors = true;
                }
                
                else
                {
                    result->float64 = (f64)BigInt_ToI64(val.big_int);
                }
            }
            
            else
            {
                ASSERT(dst == Type_F32);
                
                if (BigInt_IsGreater(val.big_int, BigInt_FromU64((u64)1 << 24)))
                {
                    //// ERROR: untyped integer is too large to fit into f32
                    encountered_errors = true;
                }
                
                else if (BigInt_IsLess(val.big_int, BigInt_FromU64((u64)1 << 24)))
                {
                    //// ERROR: untyped integer is too small to fit into f32
                    encountered_errors = true;
                }
                
                else
                {
                    result->float32 = (f32)BigInt_ToI64(val.big_int);
                }
            }
        }
        
        else
        {
            //// ERROR: Cannot coerce untyped integer to x
            encountered_errors = true;
        }
    }
    
    else if (src == Type_UntypedFloat)
    {
        NOT_IMPLEMENTED;
        //Type_Any,
        
        if (Type_IsIntegral(dst))
        {
            if (!BigFloat_IsEqual(val.big_float, BigFloat_Truncate(val.big_float)))
            {
                //// ERROR: untyped float cannot be truncated losslessly
                encountered_errors = true;
            }
            
            else
            {
                if (Type_IsSignedInteger(dst))
                {
                    u64 max = ~(u64)0 >> (65 - 8*Type_Sizeof(dst));
                    i64 min = -(i64)((u64)1 << (8*Type_Sizeof(dst) - 1));
                    
                    if (BigFloat_IsGreater(val.big_float, BigFloat_FromU64(max)))
                    {
                        //// ERROR: untyped float is too large
                        encountered_errors = true;
                    }
                    
                    else if (BigFloat_IsLess(val.big_float, BigFloat_FromI64(min)))
                    {
                        //// ERROR: untyped float is too small
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        result->int64 = BigFloat_ToI64(val.big_float) & BYTE_MASK(Type_Sizeof(dst));
                    }
                }
                
                else
                {
                    if (BigFloat_IsGreater(val.big_float, BigFloat_FromU64(BYTE_MASK(Type_Sizeof(dst)))))
                    {
                        //// ERROR: untyped float is too large
                        encountered_errors = true;
                    }
                    
                    else if (BigFloat_IsLess(val.big_float, BigFloat_0))
                    {
                        //// ERROR: untyped float is too small
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        result->uint64 = BigFloat_ToU64(val.big_float) & BYTE_MASK(Type_Sizeof(dst));
                    }
                }
            }
        }
        
        else if (Type_IsFloating(dst))
        {
            if (dst == Type_F64)
            {
                NOT_IMPLEMENTED;
            }
            
            else
            {
                ASSERT(dst == Type_F32);
                
                NOT_IMPLEMENTED;
            }
        }
        
        else
        {
            //// ERROR: Cannot coerce untyped float to x
            encountered_errors = true;
        }
    }
    
    else if (src == Type_UntypedBool)
    {
        NOT_IMPLEMENTED;
        //Type_Any,
        
        if (Type_IsBoolean(dst))
        {
            result->boolean = val.boolean;
        }
        
        else
        {
            //// ERROR: Cannot coerce untyped bool to x
            encountered_errors = true;
        }
    }
    
    else if (src == Type_UntypedChar)
    {
        NOT_IMPLEMENTED;
        //Type_Any,
        
        if (Type_IsBoolean(dst))
        {
            result->boolean = (val.uint64 != 0);
        }
        
        else if (Type_IsIntegral(dst) || dst == Type_Byte)
        {
            if (Type_IsSignedInteger(dst))
            {
                u64 max = ~(u64)0 >> (65 - 8*Type_Sizeof(dst));
                
                if (val.uint64 < max)
                {
                    //// ERROR: untyped char is too large
                    encountered_errors = true;
                }
                
                else
                {
                    result->int64 = val.uint64 & BYTE_MASK(Type_Sizeof(dst));
                }
            }
            
            else
            {
                if (val.uint64 > BYTE_MASK(Type_Sizeof(dst)))
                {
                    //// ERROR: untyped integer is too large
                    encountered_errors = true;
                }
                
                else
                {
                    result->uint64 = val.uint64 & BYTE_MASK(Type_Sizeof(dst));
                }
            }
        }
        
        else if (Type_IsFloating(dst))
        {
            if (dst == Type_F64) val.float64 = (f64)val.uint64;
            else
            {
                ASSERT(dst == Type_F32);
                
                if (val.uint64 > ((u64)1 << 24))
                {
                    //// ERROR: untyped char is too large to fit into f32
                    encountered_errors = true;
                }
                
                else
                {
                    result->float32 = (f32)val.uint64;
                }
            }
        }
        
        else
        {
            //// ERROR: Cannot coerce untyped integer to x
            encountered_errors = true;
        }
    }
    
    else
    {
        ASSERT(src == Type_UntypedString);
        NOT_IMPLEMENTED;
        //Type_Any,
        
        if (dst == Type_String && dst == Type_Cstring)
        {
            result->string = val.string;
        }
        
        else
        {
            //// ERROR: Cannot coerce untyped string to x
            encountered_errors = true;
        }
    }
    
    return !encountered_errors;
}

internal bool
ConstVal_ConvertToCommonType(Type_ID t0, Const_Val v0, Type_ID t1, Const_Val v1, Type_ID* common_type, Const_Val* v0_out, Const_Val* v1_out)
{
    bool encountered_errors = false;
    
    // NOTE: init for convenience
    *common_type = Type_NoType;
    *v0_out = v0;
    *v1_out = v1;
    
    if (Type_IsTyped(t0) && Type_IsTyped(t1))
    {
        if (t0 == t1) *common_type = t0;
        else          *common_type = Type_NoType;
    }
    
    else if (Type_IsTyped(t0) ^ Type_IsTyped(t1))
    {
        if (Type_IsTyped(t1))
        {
            *common_type = t1;
            
            if (!ConstVal_CoerceUntypedToType(t0, t1, v0, v0_out))
            {
                //// ERROR
                encountered_errors = true;
            }
        }
        
        else
        {
            *common_type = t0;
            
            if (!ConstVal_CoerceUntypedToType(t1, t0, v1, v1_out))
            {
                //// ERROR
                encountered_errors = true;
            }
        }
    }
    
    else
    {
        if (t0 == Type_UntypedInt && t1 == Type_UntypedFloat ||
            t1 == Type_UntypedInt && t0 == Type_UntypedFloat)
        {
            *common_type = Type_UntypedFloat;
            
            if (t0 == Type_UntypedFloat) v1_out->big_float = BigFloat_FromBigInt(v1.big_int);
            else                         v0_out->big_float = BigFloat_FromBigInt(v0.big_int);
        }
        
        if (t0 == Type_UntypedChar && t1 == Type_UntypedFloat ||
            t1 == Type_UntypedChar && t0 == Type_UntypedFloat)
        {
            *common_type = Type_UntypedFloat;
            
            if (t0 == Type_UntypedFloat) v1_out->big_float = BigFloat_FromF64((f64)v1.uint64);
            else                         v0_out->big_float = BigFloat_FromF64((f64)v0.uint64);
        }
        
        else if (t0 == Type_UntypedInt  && t1 == Type_UntypedChar ||
                 t0 == Type_UntypedChar && t1 == Type_UntypedInt)
        {
            *common_type = Type_UntypedChar;
            
            Big_Int i;
            
            if (t0 == Type_UntypedInt)
            {
                i          = v0.big_int;
                v0_out->uint64 = BigInt_ToU64(v0.big_int);
            }
            
            else
            {
                i          = v1.big_int;
                v1_out->uint64 = BigInt_ToU64(v1.big_int);
            }
            
            if (BigInt_IsLess(i, BigInt_0))
            {
                //// ERROR: Cannot coerce negative untyped integer to untyped char
                encountered_errors = true;
            }
            
            else if (BigInt_IsGreater(i, BigInt_FromU64(U32_MAX)))
            {
                //// ERROR: untyped integer is too large too fit into untyped char
                encountered_errors = true;
            }
        }
        
        else if (t0 == Type_UntypedInt && t1 == Type_UntypedBool ||
                 t1 == Type_UntypedInt && t0 == Type_UntypedBool)
        {
            *common_type = Type_UntypedBool;
            
            if (t0 == Type_UntypedInt) v0_out->boolean = !BigInt_IsEqual(v0.big_int, BigInt_0);
            else                       v1_out->boolean = !BigInt_IsEqual(v1.big_int, BigInt_0);
        }
        
        else if (t0 == Type_UntypedChar && t1 == Type_UntypedBool ||
                 t1 == Type_UntypedChar && t0 == Type_UntypedBool)
        {
            *common_type = Type_UntypedBool;
            
            if (t0 == Type_UntypedChar) v0_out->boolean = (v0.uint64 != 0);
            else                        v1_out->boolean = (v1.uint64 != 0);
        }
    }
    
    return !encountered_errors;
}

internal Const_Val
ConstVal_CastTo(Const_Val val, Type_ID type)
{
    Const_Val result = {0};
    
    NOT_IMPLEMENTED;
    
    return result;
}