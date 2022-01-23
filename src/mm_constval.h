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